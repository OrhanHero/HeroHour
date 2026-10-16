"""Read-only candidate evidence; never deserialize unsupported cooked exports."""
import argparse
import hashlib
import json
from pathlib import Path
from inspect_tempest_map_offline import Reader, inspect, inspect_unversioned_prefix


def inspect_names(path):
    with path.open('rb') as stream:
        data = stream.read(16 * 1024 * 1024)
    source = Reader(data)
    source.take(24)
    source.take(source.integer() * 20)
    header_size = source.integer()
    source.string()
    source.integer()
    count, offset = source.integer(), source.integer()
    if not 0 <= count <= 200000 or not source.offset <= offset < header_size:
        raise ValueError('Invalid name-table bounds')
    names, table = [], Reader(data, offset)
    for _ in range(count):
        names.append(table.string())
        table.take(4)
    if table.offset > header_size:
        raise ValueError('Name table extends beyond header')
    return names


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--content', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    if args.output.resolve().is_relative_to(args.content.resolve()):
        raise ValueError('Output must be outside the source installation')
    packages = [
        '/Game/Tempest/Props/Faction_Containers/SM_GDF_Container',
        '/Game/Tempest/Props/Faction_Containers/Materials/MI_GDF_Container_01',
        '/Game/Tempest/Props/GDFCutoffs/SM_AirtowerDetail_01',
        '/Game/Tempest/Environments/Temperate/Foliage_New/Meshes/SM_Temperate_HeatherPlant_01',
    ]
    reports = []
    for package in packages:
        path = args.content.joinpath(*package.split('/')[2:]).with_suffix('.uasset')
        try:
            detail = inspect(path)
        except ValueError:
            detail = inspect_unversioned_prefix(path)
        sidecars = []
        for sidecar in sorted(path.parent.glob(path.stem + '.*')):
            with sidecar.open('rb') as stream:
                digest = hashlib.file_digest(stream, 'sha256').hexdigest()
            sidecars.append(dict(path=str(sidecar), bytes=sidecar.stat().st_size, sha256=digest))
        reports.append(dict(package=package, path=str(path), summary=detail,
                            name_table_package_references=[n for n in inspect_names(path)
                                                           if n.startswith('/Game/')],
                            sidecars=sidecars))
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(dict(
        schema='HeroHour.TempestTransferCandidate.v1', packages=reports,
        limitation='Cooked mesh references are validated name-table entries, not decoded material slots or geometry.'
    ), indent=2) + '\n', encoding='utf-8')
    print(f'PASS: {len(reports)} package records -> {args.output}')


if __name__ == '__main__':
    main()
