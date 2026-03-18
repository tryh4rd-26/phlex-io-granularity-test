# Lazy Write Benchmarks

This directory contains benchmarking code comparing naive and indexed approaches for correlating hits and wires in Phlex RNTuple containers.

## Building

From the repository root:

```bash
cd build
cmake ..
make multicont_read_test lazy_index
```

Or build all tests:

```bash
cmake ..
make -j4
```

## Running

### Naive Scan Benchmark
```bash
./build/tests/LazyWrite/multicont_read_test
```

This runs an O(n×m) nested loop benchmark, scanning all wires for each hit to find EventID matches.

**Expected Output:**
- 100 events: ~6-7 seconds
- 1000 events: ~60-70 seconds

### Lazy Index Benchmark
```bash
./build/tests/LazyWrite/lazy_index
```

This builds an in-memory unordered_map index of EventID → wire entry positions, then performs O(n+m) hash-based lookups.

**Expected Output:**
- 100 events: ~39 ms total
- 1000 events: ~53 ms total

## Benchmarks

| Approach | 100 Events | 1000 Events | Speedup |
|----------|-----------|-----------|---------|
| Naive Scan | 6,816 ms | 68,049 ms | baseline |
| Lazy Index | 39 ms | 53 ms | **174-1283×** |

## Files

- `multicont_read_test.cpp` - Naive O(n²) nested loop implementation
- `lazy_index.cpp` - Optimized O(n+m) hash-based index implementation
- `CMakeLists.txt` - Build configuration

Both implementations:
- Read from actual Phlex `aos_event_perData.root` RNTuple files
- Support 100 and 1000 event scaling tests
- Include ROOT dictionary linking for HitIndividual/WireIndividual types
- Produce identical correlation counts for validation
