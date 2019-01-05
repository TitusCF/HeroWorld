#The value of copper is 1/2 that of sliver.
#If this copper coin had a weight of 10 it's value would be 0.5
#However, these coins have a weight of 2, so their value is 0.1
#10 copper coins equal 1 silver coin.
#Since there are no decimal values in CF I have used 0 as the value.
#thus converters are needed in banks (and the 'broken converter' check
#in apply.c or converter.c needs to be removed or disabled when playing
#with copper coins.
#--MikeeUSA--
Object coppercoin
name copper coin
race gold and jewels
face coppercoin.111
magicmap orange
nrof 1
type 36
material 2
materialname copper
value 0
weight 2
editable 2048
name_pl copper coins
client_type 2001
end
