# Dependency Boundaries

`ESPressio-Logging` is a context/domain abstraction library. It may depend only on lower-level ESPressio abstractions required by the Logging contract itself.

## Direct dependencies

- `ESPressio-System` — platform-neutral memory/synchronization facilities.
- `ESPressio-Observable` — the mandatory ESPressio observer implementation used only for structural Logging state changes.
- `ESPressio-Timing` — monotonic and synchronized System Clock time.

## Explicitly forbidden dependencies

The core Logging library must not depend on `ESPressio-Serial`, `ESPressio-Persistence`, `ESPressio-Sockets`, `ESPressio-Event`, platform SDKs, Arduino APIs, filesystem APIs, socket APIs, UART APIs, or concrete cryptographic implementations.

Concrete Logging Sink adapters are owned by their destination context:

- Serial Sink(s): `ESPressio-Serial`
- Persistent Sink(s): `ESPressio-Persistence`
- Socket Sink(s): `ESPressio-Sockets`

`ESPressio-Logging` defines shared behavioural vocabulary such as level/category filtering, buffering overflow policy, rolling/retention constraints, structured records, and Sink contracts. Destination libraries define how those contracts are realized.

## Encryption

Logging core exposes no raw-key storage or raw-key ownership API. Encrypted Sink compositions must consume opaque Security-owned key/provider handles from `ESPressio-Security`. Logging must never copy cryptographic keys into ordinary Logging-owned memory.

## Event integration

Logging and Event intentionally do not depend upon one another. Applications requiring Event-originated Logging provide the Event→Logging adapter at application level, avoiding a circular ESPressio dependency.
