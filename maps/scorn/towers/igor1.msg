{
  "location" : "scorn/igor1",
  "rules": [
  {
  "match" : ["*"],
  "pre" : [["quest", "scorn/Kelthior", "30"]],
  "post" : [],
  "msg" : ["Yes $you, how can I help you?"],
  "replies" : []
  },{
  "match" : ["izchucio"],
  "pre" : [["npctoken", "step", "2"]],
  "post" : [["quest", "scorn/Kelthior", "30"]],
  "msg" : ["Very nice, you know this good friend of my master!\n\nYou can now go to the second floor."],
  "replies" : []
  },{
  "match" : ["*"],
  "pre" : [["npctoken", "step", "2"]],
  "post" : [["setnpctoken", "step", "1"]],
  "msg" : ["Sorry, but that's not correct...\n\nDo you want to try again?"],
  "replies" : [["yes", "Yes, sorry, I mistook the name.", "1"], ["no", "No, my head is full of holes, I forgot the name...", "1"]]
  },{
  "match" : ["yes"],
  "pre" : [["npctoken", "step", "1"]],
  "post" : [["setnpctoken", "step", "2"]],
  "msg" : ["Very well.\n\nThen tell me the name of Scorn's alchemist, please."],
  "replies" : []
  },{
  "match" : ["no"],
  "pre" : [["npctoken", "step", "1"]],
  "post" : [["setnpctoken", "step", "0"]],
  "msg" : ["Then come back when you're not so busy."],
  "replies" : []
  },{
  "match" : ["*"],
  "pre" : [],
  "post" : [["setnpctoken", "step", "1"]],
  "msg" : ["So, $you, do you still wish to talk to my master?"],
  "replies" : [["yes", "Yes, of course.", "1"], ["no", "Not no, I'm too busy.", "0"]]
  }
]}
