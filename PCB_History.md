## Rev A

Prototype.

### Issues

1.  **U2 - Incorrect pinout**

    * Traces to pins 5-8 should be cut
    * Connect U2-5 to MOSI (U1-17)  
    * Connect U2-6 to SCK (U1-19)  
    * Connect U2-7 to VBat  
    * Connect U2-8 to 5V

*   **U22/U20 - Layout error**

    U20 and U22 share SER outputs with U1-4, this configuration is not usable.

    * Trace between U22-9 and U20-9 should be cut.
    * U21-10 connection to ground should be cut.
    * Connect U22-9 to U21-10.

*   **U2 - CS floating**

    CS is not pulled high and is floating when not driven by U1. May interfere with flashing via ICSP.

    Connect U1-1 to 5V via 10K resistor.

*   **ICSP Header - Incorrect footprint used**

    Pinout not compatible with standard ICSP header.

*   **R4 & R5 - incorrect value**

    Use 270-330 Ohm instead of stated 10K.