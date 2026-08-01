#include <terminal/terminal.hpp>

#include <gen/err.hpp>
#include <gen/serial.hpp>
#include <gen/vec.hpp>
#include <gen/map.hpp>
#include <gen//reboot.hpp>

#include <sys/IDT/idt.hpp>
#include <sys/GDT/gdt.hpp>

#include <mem/alloc.hpp>
#include <mem/utils.hpp>

#include <drivers/storage/driver.hpp>
#include <drivers/storage/storage.hpp>
#include <drivers/fs/ext2/ext2.hpp>
#include <drivers/fs/FAT32/fat.hpp>
#include <drivers/fs/fs.hpp>
#include <drivers/fs/file.hpp>

#include <str.hpp>

typedef void (*ctor_t)();

extern "C" ctor_t ctorsStart[];
extern "C" ctor_t ctorsEnd[];

void callGlobalConstructors() {
    // .ctors is stored in REVERSE order, walk backwards
    for (ctor_t* ctor = ctorsEnd - 1; ctor >= ctorsStart; ctor--) {
        if (*ctor != (ctor_t)-1) { // skip sentinel if present
            (*ctor)();
        }
    }
}

auto disableHardwareCursor() -> void {
    outb(0x3D4, 0x0A);
    outb(0x3D5, 0x20);
}

//TODO: i32 inst of int32_t
//TODO: some files still use <returnType> fn(...) instd of auto fn(...) -> <returnType>
extern "C" { // Disable name mangling

auto kernelMain() -> void {
    Terminal::init();
    disableHardwareCursor();

    ioInit();

    Terminal::writeStr("RivBoot worked yay\n");

    kassrt(Serial::init() == 0, "Unable to initalize serial");

    Gdt::init();
    Idt::init();

    KernelAllocator::init();

    callGlobalConstructors();

    Storage::init();

    FileSystem::init();

    fd_t stdout = FileSystem::open("/dev/stdout");
    char* msg = (char*) "WRITE\n";
    FileSystem::write(stdout, msg, strlen(msg));

    fd_t bootf = FileSystem::open("/boot/sector=0"); 
    if (!bootf) kpanic("BootFile not found???");
    char* buf = (char*) KernelAllocator::alloc(512);
    memset(buf, 0, 512);
    FileSystem::read(bootf, buf, 512);

    Terminal::printf("%s\n", buf);

    Terminal::printf("End of kernel reached");
    for (;;) ;
}

} // extern "C"



/*src/boot/enter.c: In function 'startBoot':                                                                                                                                  
src/boot/enter.c:70:23: warning: 'sectorInd' may be used uninitialized in this function [-Wmaybe-uninitialized]                                                             
   70 |     for (u32 i = 0; i < kernelSectors; i++) {                                                                                                                       
      |                     ~~^~~~~~~~~~~~~~~                                                                                                                               
In file included from src/terminal/terminal.hpp:2,                                                                                                                          
                 from src/kernel.cpp:1:                                                                                                                                     
src/gen/serial.hpp: In static member function 'static int Serial::init()':                                                                                                  
src/gen/serial.hpp:30:5: warning: no return statement in function returning non-void [-Wreturn-type]                                                                        
   30 |     }                                                                                                                                                               
      |     ^                                                                                                                                                               
src/gen/serial.hpp: In static member function 'static char Serial::read()':                                                                                                 
src/gen/serial.hpp:46:5: warning: no return statement in function returning non-void [-Wreturn-type]                                                                        
   46 |     }                                                                                                                                                               
      |     ^                                                                                                                                                               
src/gen/serial.hpp: In static member function 'static void Serial::write(char)':                                                                                            
src/gen/serial.hpp:56:28: warning: unused parameter 'a' [-Wunused-parameter]                                                                                                
   56 |     static auto write(char a) -> void {                                                                                                                             
      |                       ~~~~~^                                                                                                                                        
In file included from src/terminal/terminal.hpp:2,                                                                                                                          
                 from src/gen/err.hpp:2,                                                                                                                                    
                 from src/sys/IDT/idt.hpp:4,                                                                                                                                
                 from src/sys/IDT/idt.cpp:2:                                                                                                                                
src/gen/serial.hpp: In static member function 'static int Serial::init()':                                                                                                  
src/gen/serial.hpp:30:5: warning: no return statement in function returning non-void [-Wreturn-type]                                                                        
   30 |     }                                                                                                                                                               
      |     ^                                                                                                                                                               
src/gen/serial.hpp: In static member function 'static char Serial::read()':                                                                                                 
src/gen/serial.hpp:46:5: warning: no return statement in function returning non-void [-Wreturn-type]                                                                        
   46 |     }                                                                                                                                                               
      |     ^                                                                                                                                                               
src/gen/serial.hpp: In static member function 'static void Serial::write(char)':                                                                                            
src/gen/serial.hpp:56:28: warning: unused parameter 'a' [-Wunused-parameter]                                                                                                
   56 |     static auto write(char a) -> void {                                                                                                                             
      |                       ~~~~~^      */
