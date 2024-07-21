begin
flag: EQU 0
fim: extern
copy zero,temp
input final

IF flag
teste: add puta
load final
jmpz raiz
inic: load temp
testando: add one 
store temp
mul temp
sub final
jmpn erro
jmpp inic
raiz: ;aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
;aaaaaaaaaaaaaaaaa       testando comentarios entre labels e instruções
output temp
jmp fim
erro: output minus
jmp fim
minus: const -1
zero: const 0
one: const 1
final: SPACE
temp: SPACE
end