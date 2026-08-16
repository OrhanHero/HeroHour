# Pull Request Template

## Description
Brief description of changes

## Type of Change
- [ ] Feature (new functionality)
- [ ] Bug Fix (fixes an issue)
- [ ] Documentation (docs only)
- [ ] Refactor (code restructuring)
- [ ] Performance (optimization)
- [ ] Test (adding/fixing tests)
- [ ] Chore (maintenance, deps, config)
- [ ] Build (build system changes)
- [ ] CI (CI/CD changes)

## Related Issues
Fixes #(issue number)
Closes #(issue number)
Relates to #(issue number)

## Component(s) Affected
- [ ] simulation (F#)
- [ ] rendering (UE5)
- [ ] gameplay-ability (GAS)
- [ ] ai
- [ ] network
- [ ] data
- [ ] ui
- [ ] audio
- [ ] build
- [ ] ci
- [ ] docs
- [ ] tooling
- [ ] config

## Testing
### Unit Tests
- [ ] Added new unit tests
- [ ] Updated existing unit tests
- [ ] All unit tests pass locally

### Integration Tests
- [ ] Added integration tests
- [ ] Integration tests pass

### Manual Testing
- [ ] Tested in Editor (PIE)
- [ ] Tested in Standalone
- [ ] Tested multiplayer (2+ players)
- [ ] Tested dedicated server
- [ ] Tested AI behavior

### Performance
- [ ] Profiled (UE Insights / dotnet trace)
- [ ] No regression > 5%
- [ ] Memory stable (no leaks)

## Checklist
### Code Quality
- [ ] Code follows style guidelines (fantomas / clang-format)
- [ ] No compiler warnings (or justified with `#pragma warning`)
- [ ] Self-documenting code (clear names, types)
- [ ] Complex logic commented (why, not what)
- [ ] No `TODO`/`FIXME` left (or linked to issue)

### Architecture
- [ ] Follows project architecture (simulation/rendering separation)
- [ ] Data-driven where appropriate (data assets, not hardcoded)
- [ ] GAS patterns followed for abilities/effects
- [ ] Network replication safe (no client-authoritative state)
- [ ] F#: Functional style, Result types, no exceptions for control flow

### Documentation
- [ ] CHANGELOG.md updated
- [ ] README/docs updated if needed
- [ ] XML/Doxygen comments for public APIs
- [ ] Architecture docs updated if structural change

### Security
- [ ] No secrets/keys in code
- [ ] Input validation on all external data
- [ ] No unsafe deserialization

### Dependencies
- [ ] No unnecessary new dependencies
- [ ] Dependencies pinned to specific versions
- [ ] License compatibility verified

## Screenshots / Videos
| Before | After |
|--------|-------|
| ![before](url) | ![after](url) |

## Additional Notes
Anything else reviewers should know?