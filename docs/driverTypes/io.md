# I/O Drivers

## Basics

A I/O driver will contain Input/Output behaviour (as you could guessing the name). 

## Exposes in section "custom"

The 'custom' section will contain

- Kind=Input.USB.Keyboard|Input.PS2.Keyboard|Input.Any.Keyboard|Output.Serial|etc.: Specifies the kind of system this driver supports
- OverrideBaseInt=true|false: Should this driver be able to get the immediate interrupt or the OS just sending a signal? 
- Priority=int: specifies the priority of this driver. It is recommended for a wildcard driver (E.G.: Input.Any.Keyboard) to have this set lower (specifically 0). Unstable releases should have 1 and stable releases 2. 

## Field kind

### Targets

#### Output

* Output.Serial => Serial interface
* Output.InitConsoleTextRender => How to render text in InitConsole.
* Output.GPU => GPU driver for either a Vulcan, OpenGL or [VGOD device](../vgod.md).
* Output.Monitor => Allows for interacting with monitors (NOTE: VGOD driver depends on this)
* Output.Any.USB => Output USB devices 

#### Input

* Input.USB.Keyboard => USB keyboard
* Input.PS2.Keyboard => PS2 keyboard

### Functions

#### Output.Serial

~~~Rust
// Writes a string s to serial console.
write(str s )
~~~

#### Output.InitConsoleTextRender

~~~Rust
// Writes a ASCII Char to the screen. At position
writecAt(u8 c )
// Sets the cursor to position Vector2(x, y)
setcurspos(u16 x, u16 y )
// Clear the screen 
cls( )
~~~

#### Output.GPU

~~~Rust
// Returns the kind of GPU driver (Vulcan, OpenGL or VGOD)
getKind( )
//TODO: Spec this (further) lmao
// Returns a struct that provides a communication layer for Vulcan OpenGL or VGOD.
getCommStruct( )
~~~

#### Output.Monitor

~~~Rust
// Returns the amount of monitors
getMonitorCount( )
struct __packed__ MonitorData { 
    str name;
    u16 xres;
    u16 yres;
    u16 hz;
}
// Gets the monitor data by an ID (NOTE: The ID is just n where n < getMonitorCount())
getMonitorInfo(u16 id )
~~~
#### Output.Any.USB

No spec

#### Input.USB.Keyboard

~~~Rust
// Handler for the keyboard. 
handlr(ptr USBfrm)
~~~

#### Input.PS2.Keyboard

~~~Rust
// Handler for the keyboard
handlr(ptr PS2frm)
~~~