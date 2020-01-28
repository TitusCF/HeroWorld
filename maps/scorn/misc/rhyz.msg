{
  "rules": [
  {
  "match" : ["*"],
  "pre" : [["quest", "scorn/Rhyz", "=40"]],
  "post" : [],
  "msg" : ["Hello $you, nice to see you again."]
  }, {
  "match" : ["yes"],
  "pre" : [["quest", "scorn/Rhyz", "10"], ["npctoken", "state", "1"], ["item", "potion of gem essence"]],
  "post" : [["setnpctoken", "state", "0"], ["giveitem", "rhyzian amulet"], ["quest", "scorn/Rhyz", "40"], ["takeitem", "potion of gem essence"]],
  "msg" : ["Great! Now I can continue with my experiments!\n\nHere, take this amulet as a token of my gratitude."]
  }, {
  "match" : ["yes"],
  "pre" : [["quest", "scorn/Rhyz", "10"], ["npctoken", "state", "1"]],
  "post" : [["setnpctoken", "state", "0"]],
  "msg" : ["Hum, where is it then?"]
  }, {
  "match" : ["*"],
  "pre" : [["npctoken", "state", "1"]],
  "post" : [["setnpctoken", "state", "0"]],
  "msg" : ["Well, come back when you'll have it, then, please."]
  }, {
  "match" : ["hi", "hello"],
  "pre" : [["quest", "scorn/Rhyz", "10"]],
  "post" : [["setnpctoken", "state", "1"]],
  "msg" : ["Welcome back $you, did you get the potion from my friend?"],
  "replies" : [["yes", "Yes, I got it.", "1"], ["no", "No, not yet, sorry.", "0"]]
  }, {
  "match" : ["*"],
  "pre" : [["quest", "scorn/Rhyz", "10"]],
  "post" : [],
  "msg" : ["*grumble*"]
  }, {
  "match" : ["yes"],
  "pre" : [["npctoken", "gem", "2"]],
  "post" : [["setnpctoken", "gem", "0"], ["giveitem", "empty bottle"], ["quest", "scorn/Rhyz", "10"]],
  "msg" : ["Thank you so much. I think he is somewhere in The Frolicing Mermaid in Navar. Bring this bottle to him, tell him that Terrance send you, and ask him to fill it for me. As a reward I'll let you try out one of the amulets."]
  }, {
  "match" : ["*"],
  "pre" : [["npctoken", "gem", "2"]],
  "post" : [["setnpctoken", "gem", "0"]],
  "msg" : ["Ha well, maybe another time then. Take care."]
  }, {
  "match" : ["amulet"],
  "pre" : [["npctoken", "gem", "1"]],
  "post" : [["setnpctoken", "gem", "2"]],
  "msg" : ["I am currently working with a fellow mage in Navar who has a similar gem, to make an amulet which can indicate the distance and direction to each gem, and hence to Scorn and Navar. Right now I need a special golden potion delivered from him, containing the essence of the magical energies of his gem. Can you help get some from him?"],
  "replies" : [["yes", "Sure, what do you want me to do?", "1"], ["no", "I'm sorry, I'm quite busy for now.", "1"]]
  }, {
  "match" : ["gem", "purple", "desk"],
  "pre" : [],
  "post" : [["setnpctoken", "gem", "1"]],
  "msg" : ["The gem in my desk was a work started by my father. It was made to have a strong and distinct magical energy which can be detected from a long way away. I am currently working on amulets to work with the gem."],
  "replies" : [["amulet", "What kind of amulets?", "2"]]
  }, {
  "match" : ["rhyz"],
  "pre" : [],
  "post" : [],
  "msg" : ["I am a descendent of the great Entorinaas Durdennis Rhyz. A long time ago, he set out to create the perfect \"warriard\", one highly adept with both combat and magic, however he failed. Recently I have come to believe that modern adventurers are becoming much like the \"warriards\"  that my ancestor believed in, so I have been trying to continue his vision by encouraging adventurers."]
  }, {
  "match" : ["hide"],
  "pre" : [],
  "post" : [],
  "msg" : ["I am hiding by instructions from the king of Scorn, he fears what may happen if his enemies get some of my works."]
  }, {
  "match" : ["hello"],
  "pre" : [],
  "post" : [],
  "msg" : ["Hello. I am Terrance Rhyz. How may I help you?"],
  "replies" : [["hide", "Why are you hiding in this place?", "2"], ["rhyz", "I'm sorry, but I'm not familiar with your name...", "0"]]
  }, {
  "match" : ["*"],
  "pre" : [],
  "post" : [],
  "msg" : ["*grumble*"]
  }
]}
