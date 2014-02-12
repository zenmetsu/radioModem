This is a work in progress.

BPSK31 demodulation is possible with this code and a Maple type device. Code compiles for Arduino and Teensy.

PSK31 audio frequency should be near 1360Hz which is the halfway point between the mark and space frequencies of RTTY. This was chosen so as to minimize the need to change the frontend bandpass filtering when changing modes. You should amplify your signal with an opamp prior to feeding it through a DC blocking capacitor to your analog input pin.

Notes on architectures: I primarily develop on the Maple and use their IDE. The Maple IDE uses .pde instead of .ino files. You will likely need to rename the .pde to .ino for Teensy and Arduino. That said, here are a few notes regarding the varios platforms.

Maple: Runs fine as-is. Haven't tested the Serial output, and will need to change Serial.blah to SerialUSB.blah since the Maple uses Serial.blah for hardware serial devices and not the Serial-over-USB peripheral.

Teensy: Runs fine as-is.  Verified demodulation on Teensy 3.1 and 3.0.

Trying to use Adafruit's OLED/GFX drivers with the Teensy results in much warble garble and will not work at the moment. I ported these to run on the Maple just fine, but since Teensy uses the Arduino IDE and not their own separate IDE as the Maple does, I cannot keep two copies of the GFX library available to the Arduino IDE. A proper architecture-aware port of these libraries needs to be developed.

Arduino: Alas, poor Arduino. I have never managed to get analog reads to work fast enough to demodulate a BPSK31 signal. The problem is that the Arduino's analogRead takes something like 50 bazillion clock cycles and I cannot quite get it fast enough to do 8kHz sampling of the incoming signal... the best that I can manaage is about 3.1kHz. There may be a way to do it from a lowlevel standpoint, but I will have to try working on that at another time.

Next steps:

TX/RX windows on OLED display. Not sure what I will do about the serial output. :)

Modulation. We need output! Will probably use 4 digital pins through a resistor ladder to generate analog output. Will work on getting native DAC for Teensy3.1 working, as well as STM32F4 microcontrollers once I port the code over to those.

AT/PS2 keyboard input once modulation is added.

73s KF7IJB
