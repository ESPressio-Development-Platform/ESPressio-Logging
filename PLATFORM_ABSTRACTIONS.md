# Platform Abstractions

ESPressio-Logging contains no direct hardware/platform calls.

- Time is obtained only through ESPressio-Timing clocks.
- Synchronization is obtained only through ESPressio-System synchronization abstractions.
- Observer notification is obtained only through ESPressio-Observable.
- Concrete Serial, Persistence, Socket, encryption, filesystem, network, and device/platform behaviours are deliberately outside this repository.

Any future Logging requirement that would otherwise require a direct platform API must first be represented through the appropriate ESPressio abstraction layer.
