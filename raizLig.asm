begin          ;quando ligado com o codigo end.asm ele deve dar a raiz quadrada de um numero caso for um quadrado perfeito
flag: EQU 0
fim: extern
copy zero,temp
input final
IF flag
teste: add varnaodef
load final
jmpz raiz
inic: 
load temp
testando: add one 
store temp
mul temp
sub final
jmpz raiz
jmpn inic
jmpp erro
raiz: output temp
jmp fim
erro: output minus
jmp fim
minus: const -1
zero: const 0
one: const 1
final: SPACE
temp: SPACE
end