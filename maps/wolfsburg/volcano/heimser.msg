{
  "rules": [
  {
  "match" : ["yes"],
  "pre" : [["quest", "wolfsburg/Heimser", "=40"]],
  "post" : [["quest", "wolfsburg/Heimser", "50"], ["giveknowledge", "alchemy:2:2703:NONE"]],
  "msg" : ["Ha, of course, thank you very much.\n\nSo, here is your reward.\n\nYou can make holy symbols, in a thaumaturgy desk, with a demon's icor and a messenger's head."]
  }, {
  "match" : ["*"],
  "pre" : [["quest", "wolfsburg/Heimser", "=40"]],
  "post" : [],
  "msg" : ["So, did you ask Lordil?"],
  "replies" : [["yes", "Yes, you should melt the crystals.", "0"]]
  }, {
  "match" : ["yes"],
  "pre" : [["quest", "wolfsburg/Heimser", "=30"]],
  "post" : [],
  "msg" : ["Hum, somehow I doubt it."]
  }, {
  "match" : ["no"],
  "pre" : [["quest", "wolfsburg/Heimser", "=30"]],
  "post" : [],
  "msg" : ["*grumph*"]
  }, {
  "match" : ["*"],
  "pre" : [["quest", "wolfsburg/Heimser", "=30"]],
  "post" : [],
  "msg" : ["So, did you ask Lordil?"]
  }, {
  "match" : ["yes"],
  "pre" : [["quest", "wolfsburg/Heimser", "=20"], ["item", "mithril crystal", "10"]],
  "post" : [["takeitem", "mithril crystal", "10"], ["quest", "wolfsburg/Heimser", "30"]],
  "msg" : ["Thank you.\n\nHum, I don't remember whether I should melt them, or break them in pieces. Say, could you ask my friend Lordil, in Navar, about that?"]
  }, {
  "match" : ["yes"],
  "pre" : [["quest", "wolfsburg/Heimser", "=20"]],
  "post" : [],
  "msg" : ["Where are they?"]
  }, {
  "match" : ["no"],
  "pre" : [["quest", "wolfsburg/Heimser", "=20"]],
  "post" : [],
  "msg" : ["All right..."]
  }, {
  "match" : ["*"],
  "pre" : [["quest", "wolfsburg/Heimser", "=20"]],
  "post" : [],
  "msg" : ["Hello $you, did you get 10 mithril crystals?"],
  "replies" : [["yes", "Yes, here they are.", "1"], ["no", "Not yet, sorry.", "1"]]
  }, {
  "match" : ["yes"],
  "pre" : [["npctoken", "status", "ask"]],
  "post" : [["quest", "wolfsburg/Heimser", "20"]],
  "msg" : ["Thank you very much. I'll of course reward you, be sure of that."]
  }, {
  "match" : ["no"],
  "pre" : [["npctoken", "status", "ask"]],
  "post" : [["setnpctoken", "status", "0"]],
  "msg" : ["Ha well..."]
  }, {
  "match" : ["like"],
  "pre" : [["npctoken", "status", "like"]],
  "post" : [["setnpctoken", "status", "ask"]],
  "msg" : ["About magical crystals and such.\n\nWhich remind me, I need 10 mithril crystals, would you be as kind as get them for me?"],
  "replies" : [["yes", "Ok, I guess I can do that.", "1"], ["no", "Sorry, no.", "0"]]
  }, {
  "match" : ["lab"],
  "pre" : [["npctoken", "status", "lab"]],
  "post" : [["setnpctoken", "status", "like"]],
  "msg" : ["I'm making various experiments."],
  "replies" : [["like", "Like what?", "1"]]
  }, {
  "match" : ["brother"],
  "pre" : [],
  "post" : [],
  "msg" : ["My brother Keiran and I always were pretty close. When the armies of Kurthan the Bloody took Wolfsburg, I managed to escape, but my brother didn't, and was put into slavery.  I really don't know if he's still alive or not.  That clock is the only souvenir I have from the happy times of our youth... *sigh*"]
  }, {
  "match" : ["Euthville"],
  "pre" : [],
  "post" : [],
  "msg" : ["It was an city located at the north of Scorn, where people could build wonderful machines one can only dream of nowadays.  But their thirst for power killed them; today, only ruins remain from Euthville."]
  }, {
  "match" : ["clock"],
  "pre" : [],
  "post" : [],
  "msg" : ["A pretty nice clock, he? It is really a rare antiquity.  It was built in Euthville, during the golden age of that city.  Can you imagine? That clock is hundreds of years old!  We found it in the ruins of the city long ago, and my brother fixed it.  My brother was a genius... *sigh*"]
  }, {
  "match" : ["*"],
  "pre" : [["quest", "wolfsburg/Heimser", "=10"]],
  "post" : [["setnpctoken", "status", "lab"]],
  "msg" : ["I'm Heinkel Heimser.  What do you want from an old man like me? *sigh*"],
  "replies" : [["lab", "What is your laboratory about?", "1"]]
  }, {
  "match" : ["*"],
  "pre" : [],
  "post" : [],
  "msg" : ["I'm Heinkel Heimser.  What do you want from an old man like me? *sigh*"]
  }, {
  "match" : ["*"],
  "pre" : [],
  "post" : [],
  "msg" : []
  }
]}
