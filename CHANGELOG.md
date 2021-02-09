# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]
### Changed
- Function `Random::replace_candidate()` is slightly faster

### Removed
- Removed Catch2 submodule

## [1.0.0] - 2019-08-13
### Added
- Cache class
- FIFO, LFU, LIFO, LRU, MRU and Random replacement policies
- Basic statistics with hit/miss count, eviction count, etc...
- Function wrapper to speed up expensive function calls
- 10 examples illustrating the usage of this library
- More than 96 tests and 113.000 assertions to verify the behavior of the library

[Unreleased]: https://github.com/marcizhu/Cache/compare/v1.0.0...HEAD
[1.0.0]: https://github.com/marcizhu/Cache/releases/tag/v1.0.0
