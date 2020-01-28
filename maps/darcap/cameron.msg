{
  "location" : "darcap/cameron",
  "rules": [
  {
  "match" : ["yes"],
  "pre" : [["quest", "darcap/Cameron-wine", "=60"], ["npctoken", "askedstatus", "1"]],
  "post" : [["setnpctoken", "askedstatus", "0"], ["givecontents", "wine-reward"], ["quest", "darcap/Cameron-wine", "70"]],
  "msg" : ["Oh, many many thanks!\n\nHere, take those small jewels as a reward."]
  }, {
  "match" : ["wine"],
  "pre" : [["item", "money", "18"], ["npctoken", "step", "drink"]],
  "post" : [["takeitem", "money", "18"], ["giveitem", "bottle of wine"], ["setnpctoken", "step", "0"]],
  "msg" : ["Here you go."]
  }, {
  "match" : ["wine"],
  "pre" : [["npctoken", "step", "drink"]],
  "post" : [["setnpctoken", "step", "0"]],
  "msg" : ["Sorry, friend, but you definitely need to pay for the wine."]
  }, {
  "match" : ["drink"],
  "pre" : [],
  "post" : [["setnpctoken", "step", "drink"]],
  "msg" : ["I have some fine wine, for 18 silver coins."],
  "replies" : [["wine", "I'll have some wine, please.", "0"], ["food", "And what about food?", "2"]]
  }, {
  "match" : ["food"],
  "pre" : [],
  "post" : [],
  "msg" : ["Sorry, no. But you can try Bob's Fish & Chips, south of Darcap."],
  "replies" : [["drink", "Ok, what about drinks, then?", "2"]]
  }, {
  "match" : ["*"],
  "pre" : [["quest", "darcap/Cameron-wine", "=60"]],
  "post" : [["setnpctoken", "askedstatus", "1"]],
  "msg" : ["Hello, what can I get you?\n\nSo, did you get any clue about Scartis's wine?"],
  "replies" : [["drinks", "What drinks do you have?", "2"], ["food", "Do you have anything to eat?", "2"], ["yes", "Yes, and Scartis will now have wine again!", "1"]]
  }, {
  "match" : ["ok"],
  "pre" : [["npctoken", "askedhelp", "3"]],
  "post" : [["setnpctoken", "askedhelp", "0"], ["quest", "darcap/Cameron-wine", "10"]],
  "msg" : ["Oh many thanks!\n\nScartis's shop is located south of Darcap, just near the exit."]
  }, {
  "match" : ["no"],
  "pre" : [["npctoken", "askedhelp", "3"]],
  "post" : [["setnpctoken", "askedhelp", "0"]],
  "msg" : ["Ok, well, thanks for listening to my troubles."]
  }, {
  "match" : ["sure"],
  "pre" : [["npctoken", "askedhelp", "2"]],
  "post" : [["setnpctoken", "askedhelp", "3"]],
  "msg" : ["Well, since some time, I can't get any wine from Scartis's shop, and that's worrying me. And I have too much work here to go check.\n\nWould you be as glad as go ask what happened?"],
  "replies" : [["ok", "Ok, I'll go check.", "1"], ["no", "Sorry, I'm really busy for now...", "1"]]
  }, {
  "match" : ["no"],
  "pre" : [["npctoken", "askedhelp", "2"]],
  "post" : [["setnpctoken", "askedhelp", "0"]],
  "msg" : ["Ha, no problem."]
  }, {
  "match" : ["yes"],
  "pre" : [["npctoken", "askedhelp", "1"]],
  "post" : [["setnpctoken", "askedhelp", "2"]],
  "msg" : ["Actually, I could use some help too, if you'd help me?\n\nOf course I'd reward you."],
  "replies" : [["sure", "Well, tell me, and I may be able to help you.", "1"], ["no", "Sorry, but right now I'm busy.", "1"]]
  }, {
  "match" : ["*"],
  "pre" : [["questdone", "darcap/Spike", "50"], ["quest", "darcap/Cameron-wine", "=0"]],
  "post" : [["setnpctoken", "askedhelp", "1"]],
  "msg" : ["Hello, what can I get you?\n\nSorry to ask, but didn't you help Spike get a potion for his backache?"],
  "replies" : [["drinks", "What drinks do you have?", "2"], ["food", "Do you have anything to eat?", "2"], ["yes", "Well, yes, I helped him some.", "1"]]
  }, {
  "match" : ["yes", "no"],
  "pre" : [["npctoken", "askedstatus", "1"]],
  "post" : [["setnpctoken", "askedstatus", "0"]],
  "msg" : ["Well, please let me know when you find what happened..."]
  }, {
  "match" : ["*"],
  "pre" : [["quest", "darcap/Cameron-wine", "10-50"]],
  "post" : [["setnpctoken", "askedstatus", "1"]],
  "msg" : ["Hello, what can I get you?\n\nSo, did you get any clue about Scartis's wine?"],
  "replies" : [["drinks", "What drinks do you have?", "2"], ["food", "Do you have anything to eat?", "2"], ["yes", "Yes, but I haven't solved everything yet.", "1"], ["no", "No, not yet.", "1"]]
  }, {
  "match" : ["*"],
  "pre" : [],
  "post" : [],
  "msg" : ["Hello, what can I get you?"],
  "replies" : [["drinks", "What drinks do you have?", "2"], ["food", "Do you have anything to eat?", "2"]]
  }
]}
