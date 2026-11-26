# Using this repo in STM32CubeIDE

The project is a plain Makefile project, so it imports into CubeIDE with no
generated code, no `.ioc`, and no HAL. CubeIDE supplies the compiler, `make`,
and the ST-LINK debugger.

## 1. Import

1. `git clone https://github.com/basavarajjn/STM32_utilities.git`
2. CubeIDE → **File → Import… → C/C++ → Existing Code as Makefile Project** → Next
3. *Existing Code Location*: the cloned folder
4. *Toolchain for Indexer Settings*: **MCU ARM GCC** → Finish

## 2. Choose which example to build

Right-click the project → **Properties → C/C++ Build**:

- Uncheck *Use default build command*
- Build command: `make EXAMPLE=spi_loopback` (or `EXAMPLE=spi_master_slave`)
- Build directory: `${workspace_loc:/stm32-utilities}`

Press the hammer icon (Ctrl+B). The console shows FLASH/RAM usage when the build works.

> If CubeIDE says `make: not found`, go to **Window → Preferences → C/C++ → Build → Environment** and add CubeIDE's
> bundled make and GCC folders to `PATH`. You can find them under
> `<CubeIDE>/plugins/com.st.stm32cube.ide.mcu.externaltools.make.*/tools/bin` and
> `…externaltools.gnu-tools-for-stm32.*/tools/bin`.

## 3. Flash and debug

1. **Run → Debug Configurations… → STM32 C/C++ Application** → New
2. *C/C++ Application*: `build/spi_loopback/spi_loopback.elf`
3. *Debugger* tab: Debug probe **ST-LINK (ST-LINK GDB server)**, interface **SWD**
4. Debug. The program halts at `main`. Press Resume (F8).

To read the test output, open the CubeIDE **Console → Command Shell Console → Serial Port**
on the ST-LINK VCP at 115200 baud. PuTTY, Tera Term or `screen` also work.

## Peripheral register view

Because the build uses `-g3` and the chip is identified as STM32F446RE in the debug config,
the **SFRs** view shows live SPI1/SPI2 registers (CR1, SR, DR). This is handy when a test fails.
