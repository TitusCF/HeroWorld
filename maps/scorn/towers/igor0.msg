{
  "location" : "scorn/igor",
  "rules": [
  {
  "match" : ["*"],
  "pre" : [["quest", "scorn/Kelthior", "20"]],
  "post" : [],
  "msg" : ["Hello $you, welcome back."],
  "replies" : []
  },{
  "match" : ["yes"],
  "pre" : [["npctoken", "asked", "1"], ["knowledgeknown", "alchemy:1:3829:the wise"]],
  "post" : [["quest", "scorn/Kelthior", "20"]],
  "msg" : ["Very well!\n\nYou can now get to the first floor, where my master has prepared some more challenges."],
  "replies" : []
  },{
  "match" : ["yes"],
  "pre" : [["npctoken", "asked", "1"]],
  "post" : [["setnpctoken", "asked", "0"]],
  "msg" : ["Now now, don't do guessing, really learn it."],
  "replies" : []
  },{
  "match" : ["no"],
  "pre" : [["npctoken", "asked", "1"]],
  "post" : [["setnpctoken", "asked", "0"]],
  "msg" : ["Then go and learn it."],
  "replies" : []
  },{
  "match" : ["*"],
  "pre" : [["quest", "scorn/Kelthior", "10"]],
  "post" : [["setnpctoken", "asked", "1"]],
  "msg" : ["So, do you know the formulae of the water of the wise?"],
  "replies" : [["yes", "Yes.", "1"], ["no", "No, not yet...", "1"]]
  },{
  "match" : ["how"],
  "pre" : [["npctoken", "disturb", "2"]],
  "post" : [["quest", "scorn/Kelthior", "10"]],
  "msg" : ["First, you have to know the alchemy formulae for the water of the wise."],
  "replies" : []
  },{
  "match" : ["talk"],
  "pre" : [["npctoken", "disturb", "1"]],
  "post" : [["setnpctoken", "disturb", "2"]],
  "msg" : ["My master is a knowledgeable man, and would enjoy a civilized discussion.\n\nIf you can show you are such a person, then he will not mind talking to you, even if he is busy."],
  "replies" : [["how", "And how would I show that I indeed am a knowledgeable person?", "0"]]
  },{
  "match" : ["*"],
  "pre" : [],
  "post" : [["setnpctoken", "disturb", "1"]],
  "msg" : ["Please do not disturb my master, he's busy doing some experiments."],
  "replies" : [["talk", "And what if I really want to talk to him?", "2"]]
  }
]}
