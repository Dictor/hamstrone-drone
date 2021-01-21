# hamstrone-drone
Quad-copter firmware for hamstrone. It's app on NuttX RTOS

## Requirement
For build hamstrone development environment, below programs are required.
- Cygwin + package (see [here](https://kimdictor.kr/post/embedded/intall-nuttx-nucleo/) for list of required pacakge)
- arm-non-eabi-gcc
- golang (>1.15)
- board with stm32l432kc processor (highly recommend [NUCLEO-L432KC board](https://www.st.com/en/evaluation-tools/nucleo-l432kc.html))

## Build environment
Execute [install script](https://gist.github.com/Dictor/7c91d169fe4b04a8cca2519de5d49546#gistcomment-3581782) on cygwin after above requirement is fulfilled

## Tools
Hamstrone environment proivide Makefile for often used feature. Execute `make help` in `hamstrone` directory for usage.
For additional installation instruction, see [here](https://kimdictor.kr/post/embedded/intall-nuttx-nucleo/)
