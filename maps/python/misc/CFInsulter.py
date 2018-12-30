# CFInsulter.py (formerly shakeinsult.py)
# This is an adaptation of Nick Hodges' Shakspearean Insult Generator in python.
#
# "This is available under a BSD style license.  Once I said I would never
# write any non-GPL stuff for fun.  Oh well.  Just let me know if you use it."
# http://www.zope.org/Members/tfarrell/shakeinsult
#
#
# Adapted for use in Crossfire by Todd Mitchell
#
# Please help by adding new styles of insults to this fine script.

import Crossfire
from random import choice

def Insult(style):

##SHAKESPEAR STYLE

  if style == "shakespear":
    adj1 = ['artless', 'bawdy', 'beslubbering', 'bootless', 'churlish', 'cockered', 'clouted', 'craven', 'currish', 'dankish', 'dissembling', 'droning', 'errant', 'fawning', 'fobbing', 'froward', 'frothy', 'gleeking', 'goatish', 'gorbellied', 'impertinent', 'infectious', 'jarring', 'loggerheaded', 'lumpish', 'mammering', 'mangled', 'mewling', 'paunchy', 'pribbling', 'puking', 'puny', 'qualling', 'rank', 'reeky', 'roguish', 'ruttish', 'saucy', 'spleeny', 'spongy', 'surly', 'tottering', 'unmuzzled', 'vain', 'venomed', 'villainous', 'warped', 'wayward', 'weedy', 'yeasty', 'vomiting', 'vulturous', 'contemptuous', 'groping', 'miniscule', 'quivering', 'shivering', 'trembling', 'miserable', 'licentious', 'cowering', 'sulking', 'gloating', 'murmuring', 'audacious', 'befouling', 'insolent', 'murky', 'pitiable', 'wretched', 'dolorous', 'lamentable', 'inadequate', 'contemptible', 'paltry', 'measly', 'meager', 'paltry', 'inadequate', 'insignificant', 'empty', 'inferior', 'pathetic', 'atrocious', 'execrable', 'damnable', 'repugnant', 'repulsive', 'revolting', 'repellent', 'offensive', 'disgusting', 'horrid', 'horrible', 'obscene', 'beastly', 'vile', 'abominable', 'pitiful', 'wrangled', 'whoring']
    adj2 = ['base-court', 'bat-fowling', 'beef-witted', 'beetle-headed', 'boil-brained', 'clapper-clawed','clay-brained', 'common-kissing', 'crook-pated', 'dismal-dreaming', 'dizzy-eyed', 'doghearted', 'dread-bolted', 'earth-vexing', 'elf-skinned', 'fat-kidneyed', 'fen-sucked', 'flap-mouthed', 'fly-bitten', 'folly-fallen', 'fool-born', 'full-gorged', 'guts-griping', 'half-faced', 'hasty-witted', 'hedge-born', 'hell-hated', 'idle-headed', 'ill-breeding', 'ill-nurtured', 'knotty-pated', 'milk-livered', 'motley-minded', 'onion-eyed', 'plume-plucked', 'pottle-deep', 'pox-marked', 'reeling-ripe', 'rough-hewn', 'rude-growing', 'rump-fed', 'shard-borne', 'sheep-biting', 'spur-galled', 'swag-bellied', 'tardy-gaited', 'tickle-brained', 'toad-spotted', 'unchin-snouted', 'weather-bitten', 'weather-beaten', 'mutton-eating', 'coffee-nosed', 'malodorous']
    noun = ['apple-john', 'baggage', 'barnacle', 'bladder', 'boar-pig', 'bugbear', 'bum-bailey', 'canker-blossom', 'clack-dish', 'clotpole', 'coxcomb', 'codpiece', 'death-token', 'dewberry', 'flap-dragon', 'flax-wench', 'flirt-gill', 'foot-licker', 'fustilarian', 'giglet', 'gudgeon', 'haggard', 'harpy', 'hedge-pig', 'horn-beast', 'hugger-mugger', 'joithead', 'lewdster', 'lout', 'maggot-pie', 'malt-worm', 'mammet', 'measle', 'minnow', 'miscreant', 'moldwarp', 'mumble-news', 'nut-hook', 'pigeon-egg', 'pignut', 'puttock', 'pumpion', 'ratsbane', 'scut', 'skainsmate', 'strumpet', 'varlet', 'vassal', 'whey-face', 'wagtail', 'phlegm-barrel', 'numb-skull', 'lip-infection', 'blood-clot', 'boar-tick', 'pervert']
    prefixA = ['Thou art a','Thy Mother is a', 'Thou']
    prefixAn = ['Thou art an', 'Thy Mother is an', 'Thou']

##TEXAS STYLE

  elif style == "texas":
    adj1 = ['stewpid', 'uglee', 'pea brained', 'dung-headed', 'beady-eyed', 'hatless', 'witless', 'dumb']
    adj2 = ['horse-knappin', 'hog-lickin', 'knock-kneed', 'jug-eared', 'pie-headed', 'snaggle-toothed', 'brown-nosed', 'lilly-livered' ]
    noun = ['dipshit', 'city-slicker', 'root-head', 'cow-pie', 'greenhorn', 'idgit']
    prefixA = ['Yer a','Yer Mama\'s a', 'Yew']
    prefixAn = ['Yer an', 'Yer Mama\'s an', 'Yew']

##DWARVEN STYLE

  elif style == "dwarf":
    adj1 = ['beardless', 'puny', 'shaven']
    adj2 = ['elf-footed', 'dull-axed', 'tin-shielded' ]
    noun = ['orc-lover', 'gobiln-kisser', 'tree-embracer']
    prefixA = ['You are a','Your Matron\'s a', 'You']
    prefixAn = ['You are an', 'Your Matron\'s an', 'You']

##NO STYLE
  else:
    return "No such style stupid."

  vowels = ['a', 'e', 'i', 'o', 'u']
  isvowel = 0

  rnoun = choice(noun)
  radj1 = choice(adj1)
  radj2 = choice(adj2)

  for letter in vowels:
    if (radj1[0] == letter):
      rprefix = choice(prefixAn)
      isvowel = 1
  if (isvowel == 0):
    rprefix = choice(prefixA)
  insult = "%s %s %s %s!" % (rprefix, radj1, radj2, rnoun)
  return insult


activator=Crossfire.WhoIsActivator()
whoami=Crossfire.WhoAmI()
#style of insult desired to hurl in event options
style = Crossfire.ScriptParameters() # 1 is apply event

activator.Write(Insult(style))
