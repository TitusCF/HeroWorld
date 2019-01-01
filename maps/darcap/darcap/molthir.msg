{
  "location" : "darcap/darcap/molthir",
  "rules": [
  {
  "match" : ["yes"],
  "pre" : [["quest", "darcap/Spike", "20-30"], ["item", "smaprh roots", "5"], ["npctoken", "asked_roots", "yes"]],
  "post" : [["quest", "darcap/Spike", "40"], ["takeitem", "smaprh root", "5"], ["givecontents", "pouch"], ["setnpctoken", "asked_roots", "no"]],
  "msg" : ["Great! Now I can make my potion.\n\nHere, give that to Spike, that should help him."],
  "replies" : []
  },{
  "match" : ["yes"],
  "pre" : [["quest", "darcap/Spike", "20-30"], ["npctoken", "asked_roots", "yes"]],
  "post" : [["setnpctoken", "asked_roots", "no"]],
  "msg" : ["Well, where are there, then?"],
  "replies" : []
  },{
  "match" : ["no"],
  "pre" : [["quest", "darcap/Spike", "20-30"], ["npctoken", "asked_roots", "yes"]],
  "post" : [["setnpctoken", "asked_roots", "no"]],
  "msg" : ["Well, please go to Bob's place to get them..."],
  "replies" : []
  },{
  "match" : ["*"],
  "pre" : [["quest", "darcap/Spike", "20-30"]],
  "post" : [["setnpctoken", "asked_roots", "yes"]],
  "msg" : ["So, did you get those roots?"],
  "replies" : [["yes", "Yes, I got them from Bob.", "1"], ["no", "No, not yet", "1"]]
  },{
  "match" : ["yes"],
  "pre" : [["quest", "darcap/Spike", "10"]],
  "post" : [["quest", "darcap/Spike", "20"]],
  "msg" : ["That is really generous of you, thank you!"],
  "replies" : []
  },{
  "match" : ["no"],
  "pre" : [["quest", "darcap/Spike", "10"]],
  "post" : [],
  "msg" : ["Ha well, maybe later them..."],
  "replies" : []
  },{
  "match" : ["potion"],
  "pre" : [["quest", "darcap/Spike", "10"]],
  "post" : [],
  "msg" : ["Ha yes, the famous Potion to cure back aches!\n\nUnfortunately I need some smaprh roots. Bob, at the general store in the south, should have gotten them, but right now I can't get them, I'm doing some alchemy.\n\nWould you mind getting them for me?\n"],
  "replies" : [["yes", "Ok, *sigh*, I'll go get them.", "1"], ["no", "Not now, sorry, I got things to do.", "1"]]
  },{
  "match" : ["*"],
  "pre" : [["quest", "darcap/Spike", "10"]],
  "post" : [],
  "msg" : ["Hello, and welcome to Molthir's potion shop!\n\nI hope you'll find suitable things."],
  "replies" : [["potion", "Spike sent me to get a potion for his hurting back, do you have one around?", "0"]]
  },{
  "match" : ["*"],
  "pre" : [],
  "post" : [],
  "msg" : ["Hello, and welcome to Molthir's potion shop!\n\nI hope you'll find suitable things."],
  "replies" : []
  }
]}
