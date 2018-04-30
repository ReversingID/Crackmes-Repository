# Github Users Crackme

Preserved by [Reversing.ID](https://Reversing.ID)

Original link: [CrackMe](https://github.com/NWMonster/CrackMe)

Platform: Linux/Unix

# Readme

给程序正确的参数之后将会得到正确提示

[~/Projects/CrackMe]$ ./300.bin 747568616F2C63616E20774520624520667269456E64733F
Nice!!! U got it!

如果默认运行输出Hello World!其他不给予提示

参数是flag的ascii的hex表现形式，这里可以做个hint

"hex2ascii"

里面有一个隐含的字符串"HACK_THE_PLANET"作为混淆，如果很多人误认为这是flag的情况
可以给予提示

"'HACK_THE_PLANET' is not the flag!"

