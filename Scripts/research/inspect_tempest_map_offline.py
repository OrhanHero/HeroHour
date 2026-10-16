"""Read bounded UE5.4 package tables without loading Unreal or changing assets.

Supports versioned legacy -8 / UE4 522 / UE5 1008..1013 packages only.
Does not deserialize actor properties, geometry, textures or Blueprint bytecode.
"""
import argparse
import collections
import hashlib
import json
from pathlib import Path
import struct


class Reader:
    def __init__(self, data, offset=0):
        self.data, self.offset = data, offset

    def take(self, count):
        if count < 0 or self.offset + count > len(self.data):
            raise ValueError('Read outside bounded header')
        value = self.data[self.offset:self.offset + count]
        self.offset += count
        return value

    def integer(self):
        return struct.unpack('<i', self.take(4))[0]

    def string(self):
        size = self.integer()
        if abs(size) > 100000:
            raise ValueError('Unreasonable FString length')
        if size == 0:
            return ''
        value = self.take(abs(size) * (2 if size < 0 else 1))
        terminator = b'\0\0' if size < 0 else b'\0'
        if not value.endswith(terminator):
            raise ValueError('Unterminated FString')
        return value[:-len(terminator)].decode('utf-16-le' if size < 0 else 'utf-8')


def inspect(path):
    with path.open('rb') as stream:
        data = stream.read(16 * 1024 * 1024)
    reader = Reader(data)
    if reader.take(4) != b'\xc1\x83\x2a\x9e':
        raise ValueError('Not a standard little-endian Unreal package')
    legacy, ue3, ue4, ue5, licensee = [reader.integer() for _ in range(5)]
    if legacy != -8 or ue4 != 522 or not 1008 <= ue5 <= 1013 or licensee != 0:
        raise ValueError(f'Unsupported package version: {legacy}, {ue4}, {ue5}, {licensee}')
    custom_count = reader.integer()
    if not 0 <= custom_count <= 500:
        raise ValueError('Invalid custom version count')
    reader.take(custom_count * 20)
    header_size = reader.integer()
    package_name = reader.string()
    flag_offset = reader.offset
    flags = struct.unpack('<I', reader.take(4))[0]
    name_count, name_offset = reader.integer(), reader.integer()
    soft_count, soft_offset = reader.integer(), reader.integer()
    if not flags & 0x80000000:
        reader.string()  # LocalizationId is absent with FilterEditorOnly.
    gather_count, gather_offset = reader.integer(), reader.integer()
    export_count, export_offset = reader.integer(), reader.integer()
    import_count, import_offset = reader.integer(), reader.integer()
    if not (0 < header_size <= path.stat().st_size and 0 <= name_count <= 200000
            and 0 <= import_count <= 200000 and name_offset < header_size
            and import_offset + import_count * 40 <= header_size):
        raise ValueError('Invalid package table bounds')
    if header_size > len(data):
        raise ValueError('Header exceeds the 16 MiB read boundary')
    names = []
    table = Reader(data, name_offset)
    for _ in range(name_count):
        names.append(table.string())
        table.take(4)  # Two serialized uint16 name hashes.
    name_table_end = table.offset
    if name_table_end > header_size:
        raise ValueError('Name table extends past package header')

    def fname(source):
        index, number = source.integer(), source.integer()
        if not 0 <= index < len(names) or number < 0:
            raise ValueError('Invalid import FName reference')
        return names[index] + (f'_{number - 1}' if number else '')

    imports = []
    table = Reader(data, import_offset)
    for index in range(import_count):
        start = table.offset
        class_package, class_name = fname(table), fname(table)
        outer = table.integer()
        object_name, owner_package = fname(table), fname(table)
        optional = table.integer()
        if optional not in (0, 1) or not -import_count <= outer <= export_count:
            raise ValueError('Invalid import link or optional flag')
        imports.append(dict(index=-index-1, file_offset=start,
                            class_package=class_package, class_name=class_name,
                            outer_index=outer, object_name=object_name,
                            owner_package=owner_package, optional=bool(optional)))
    for item in imports:
        chain, visited, outer = [], set(), item['outer_index']
        while outer < 0:
            if outer in visited:
                raise ValueError('Cyclic import outer chain')
            visited.add(outer)
            ancestor = imports[-outer-1]
            chain.append(ancestor['object_name'])
            outer = ancestor['outer_index']
        item['outer_chain'] = chain
    with path.open('rb') as stream:
        digest = hashlib.file_digest(stream, 'sha256').hexdigest()
    return dict(path=str(path), bytes=path.stat().st_size, sha256=digest,
                result='parsed', version=dict(legacy=legacy, ue4=ue4, ue5=ue5),
                package_name=package_name, flags_hex=f'0x{flags:08x}',
                flags_file_offset=flag_offset, cooked=bool(flags & 0x200),
                filter_editor_only=bool(flags & 0x80000000), header_size=header_size,
                name_count=name_count, name_offset=name_offset, name_table_end=name_table_end,
                export_count=export_count, export_offset=export_offset,
                import_count=import_count, import_offset=import_offset,
                soft_object_path_count=soft_count, soft_object_path_offset=soft_offset,
                imported_packages=sorted({i['object_name'] for i in imports if i['class_name']=='Package'}),
                class_imports=[i for i in imports if i['class_name'] in ('Class', 'BlueprintGeneratedClass')],
                imports=imports,
                limitations=['No export/property deserialization; imports do not prove actor instances or live behavior.',
                             'Soft paths, registry dependencies and transitive dependencies are not fully decoded.'])


def inspect_unversioned_prefix(path):
    """Read only the validated legacy -8 prefix, without inventing a UE version."""
    with path.open('rb') as stream:
        data = stream.read(65536)
    source = Reader(data)
    if source.take(4) != b'\xc1\x83\x2a\x9e':
        raise ValueError('Unsupported unversioned package magic')
    legacy, ue3, ue4, ue5, licensee = [source.integer() for _ in range(5)]
    if (legacy, ue4, ue5, licensee) != (-8, 0, 0, 0):
        raise ValueError('Not a legacy -8 unversioned prefix')
    count = source.integer()
    if not 0 <= count <= 500:
        raise ValueError('Invalid custom versions')
    source.take(count*20)
    header_size = source.integer()
    package_name = source.string()
    flag_offset = source.offset
    flags = struct.unpack('<I', source.take(4))[0]
    names, name_offset = source.integer(), source.integer()
    if not (0 < header_size <= path.stat().st_size and 0 <= names <= 200000
            and source.offset <= name_offset <= header_size):
        raise ValueError('Unversioned prefix failed independent size/offset bounds')
    with path.open('rb') as stream:
        digest = hashlib.file_digest(stream, 'sha256').hexdigest()
    return dict(sha256=digest, cooked=bool(flags & 0x200),
                filter_editor_only=bool(flags & 0x80000000), flags_hex=f'0x{flags:08x}',
                flags_file_offset=flag_offset, package_name=package_name,
                version=dict(legacy=legacy, ue4=ue4, ue5=ue5, unversioned=True),
                limitation='Only legacy -8 summary prefix decoded; no inferred current UE version or import layout.')


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--project', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    project = args.project.resolve()
    output = args.output.resolve()
    if output == project or project in output.parents:
        raise ValueError('Reports must be outside the Tempest installation')
    target = project/'Mods/SetdressingExampleMap/Content/Maps/Multiplayer/4Player/SEM'
    files = sorted(target.rglob('*.umap')) + sorted(target.rglob('*.uasset'))
    reports = []
    for path in files:
        try:
            report = inspect(path)
        except (ValueError, UnicodeError, struct.error) as error:
            report = dict(path=str(path), result='unsupported', reason=str(error))
        reports.append(report)
    references = sorted({ref for report in reports if report['result']=='parsed'
                         for ref in report['imported_packages']})
    dependencies = []
    for ref in references:
        parts = ref.strip('/').split('/')
        mount = parts[0]
        dependency = dict(package=ref, mount=mount)
        if mount in ('Script', 'Engine'):
            dependency['status'] = 'native_module' if mount=='Script' else 'engine_asset'
        else:
            content = project/'Content' if mount=='Game' else project/'Mods'/mount/'Content'
            if not content.is_dir():
                content = project/'Plugins'/mount/'Content'
            candidates = [content.joinpath(*parts[1:]).with_suffix(ext) for ext in ('.uasset','.umap')]
            asset = next((candidate for candidate in candidates if candidate.is_file()), None)
            dependency['status'] = 'found' if asset else 'not_found_in_project_mod_plugin_mounts'
            if asset:
                dependency['path'] = str(asset)
                try:
                    detail = inspect(asset)
                    dependency.update(header_result='parsed', sha256=detail['sha256'],
                                      cooked=detail['cooked'], filter_editor_only=detail['filter_editor_only'],
                                      version=detail['version'])
                except (ValueError, UnicodeError, struct.error) as error:
                    dependency.update(header_result='unsupported', reason=str(error))
                    try:
                        dependency.update(inspect_unversioned_prefix(asset))
                        dependency['header_result'] = 'summary_only_unversioned'
                    except (ValueError, UnicodeError, struct.error):
                        pass
        dependencies.append(dependency)
    summary = dict(schema='HeroHour.TempestOfflinePackageInventory.v1',
                   method='Validated UE package summary, name table and import table; no engine execution.',
                   input_root=str(target), files=len(files),
                   results=dict(collections.Counter(r['result'] for r in reports)),
                   packages=reports)
    output.mkdir(parents=True, exist_ok=True)
    (output/'Package-Imports.json').write_text(json.dumps(summary, indent=2)+'\n', encoding='utf-8')
    (output/'Direct-Dependencies.json').write_text(json.dumps(dict(
        schema='HeroHour.TempestDirectDependencies.v1', references=len(references),
        statuses=dict(collections.Counter(d['status'] for d in dependencies)),
        dependencies=dependencies,
        limitation='Direct package imports from the 43 selected files, not a complete transitive migration closure.'), indent=2)+'\n', encoding='utf-8')
    print(json.dumps(dict(files=len(files), results=summary['results'], report=str(output/'Package-Imports.json'))))


if __name__ == '__main__':
    main()
