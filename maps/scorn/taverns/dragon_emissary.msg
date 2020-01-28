{ "location" : "goths_dragon_emissary",
  "rules"    : [
  {
  "match" : ["emerald", "yes"],
  "pre" : [["quest", "scorn/DragonWhisker", "30"], ["token", "question", "emeralds"], ["item", "emeralds of great value", "7"]],
  "post" : [["settoken", "question", "0"], ["quest", "scorn/DragonWhisker", "40"],  ["takeitem", "emeralds of great value", "7"]],
  "msg" : ["The Dragonlords fear to keep the weapon too far from themselves, yet don't trust each other enough to keep it too close at hand, either.\n\n They have hidden it in a mountain eyrie near their lair: you must search the high mountains nearby.\n The Dragonlords' lair is in the mountains south of the road to the town you call Navar, there is an inn nearby run by the human known as Hermes"]
  },{
  "match" : ["emerald", "yes"],
  "pre" : [["quest", "scorn/DragonWhisker", "30"], ["token", "question", "emeralds"]],
  "post" : [["settoken", "question", "0"]],
  "msg" : ["Where are they?"]
  },{
  "match" : ["emerald", "bribe", "tell"],
  "pre" : [["quest", "scorn/DragonWhisker", "30"]],
  "post" : [["settoken", "question", "emeralds"]],
  "msg" : ["Have you got my emeralds?"],
  "replies" : [["yes", "Here are your emeralds"], ["no", "I am still looking for the emeralds you desire"]]
  },{ 
  "match" : ["bribe", "persuade"],
  "pre"   : [["quest", "scorn/DragonWhisker", "20"]],
  "post"  : [["quest", "scorn/DragonWhisker", "30"]],
  "msg"   : ["You're beginning to interest me.  I have a weakness for emeralds of great value.\n Bring me seven, and I shall tell you what you want to know."]
  },{ 
  "match" : ["weapon"],
  "pre"   : [],
  "post"  : [],
  "msg"   : ["What?  Where did you hear about that?\n Well, why would I tell you anything about this mythical 'weapon'?"]
  },{ 
  "match" : ["home"],
  "pre"   : [],
  "post"  : [],
  "msg"   : ["Their dwelling is on the western edge of the mountains southwest of Navar."]
  },{ 
  "match" : ["dragonlord", "master"],
  "pre"   : [],
  "post"  : [],
  "msg"   : ["My masters, the Dragonlords, are most puissant.  Few mortals may hope to overcome them in combat, for they wield such magics as would snuff the life of a mortal in seconds.\n No mortal can deal blows, arcane or otherwise, quickly enough to destroy the Lords, before they themselves are destroyed.\n The only way a mortal could destroy one of the Dragonlords is by some clever device, for none may face one in straight combat to the death and live.\n To even challenge a Dragonlord to combat, you would have to pass all their draconic guards.\n The last of these are Ancient Dragons, found deep in the Dragonlord's home; their teeth are spears, their claws are swords, and their breath is death!"],
  "replies" : [["home", "Where do the Dragonlords dwell?"]]
  },{ 
  "match" : ["demand"],
  "pre"   : [],
  "post"  : [],
  "msg"   : ["The Dragonlords demand in tribute:\n10 virgins per year,\n100 head of cattle,\n and 100000 pieces of gold\n Such tribute would help them express kindness by refraining from destroying your squalid little city."]
  },{ 
  "match" : ["hi", "hello", "job"],
  "pre"   : [],
  "post"  : [],
  "msg"   : ["I am the Emissary of the Dragonlords to the City of Scorn.  I have come to present the Demands of my masters.  However, your King is less than cooperative.  I should tell my masters that the city is ripe for conquer."],
  "replies" : [["demands", "What are the demands of the Dragon Lords?"], ["dragonlords", "Who are the Dragonlords?"]]
  },{ 
  "match" : ["name"],
  "pre"   : [],
  "post"  : [],
  "msg"   : ["Gramzon, Bringer-of-Ashes"]
  }
]}

