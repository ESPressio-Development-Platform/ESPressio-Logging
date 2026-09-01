# Optimisations

The initial Logging design is intentionally optimised around constrained embedded systems.

- No log payload copy occurs in the Logger/Router synchronous path.
- `std::string_view` and bounded field views avoid mandatory string/container construction.
- The Router uses fixed-capacity Sink and category-override tables rather than dynamic registries.
- Runtime filtering occurs before timestamp/message/metadata work whenever the compile-time macro path is used.
- Disabled compile-time levels preprocess to no-op statements and do not evaluate their arguments.
- Multiple Sinks receive the same lease; durable records use intrusive sharing rather than payload duplication.
- Asynchronous Sinks are expected to materialise directly into their final bounded representation where cheaper than retaining a semantic record.
- The zero-Sink path exits before locking or timestamp capture.
- Observer notification is restricted to structural Logging state changes, never ordinary record dispatch.
