{
  "location" : "darcap/talok",
  "rules": [
  {
  "match" : ["*"],
  "pre" : [["quest", "darcap/Cameron-wine", "50"]],
  "post" : [],
  "msg" : ["Hello $you, what a great day, isn't it?", "Hello $you, how are you doing?"],
  "replies" : []
  },{
  "match" : ["yes"],
  "pre" : [["quest", "darcap/Cameron-wine", "30"], ["npctoken", "step", "asked"], ["item", "Talok's scythe"]],
  "post" : [["quest", "darcap/Cameron-wine", "50"], ["takeitem", "Talok's scythe"]],
  "msg" : ["Thank you!\n\nNow I can finally get back to cutting grapes!"],
  "replies" : []
  },{
  "match" : ["yes"],
  "pre" : [["quest", "darcap/Cameron-wine", "30"], ["npctoken", "step", "asked"]],
  "post" : [["setnpctoken", "step", "start"]],
  "msg" : ["But where is it?"],
  "replies" : []
  },{
  "match" : ["no"],
  "pre" : [["quest", "darcap/Cameron-wine", "30"], ["npctoken", "step", "asked"]],
  "post" : [["setnpctoken", "step", "start"]],
  "msg" : ["Well, talk to me again when you got it, then..."],
  "replies" : []
  },{
  "match" : ["*"],
  "pre" : [["quest", "darcap/Cameron-wine", "30"]],
  "post" : [["setnpctoken", "step", "asked"]],
  "msg" : ["So, did you recover my scythe?"],
  "replies" : [["yes", "Yes, I got it!", "1"], ["no", "No, not yet...", "1"]]
  },{
  "match" : ["search"],
  "pre" : [["npctoken", "step", "mountains"]],
  "post" : [["quest", "darcap/Cameron-wine", "30"]],
  "msg" : ["Really?\n\nThat's very nice of you, thank you!"],
  "replies" : []
  },{
  "match" : ["where"],
  "pre" : [["npctoken", "step", "stolen"]],
  "post" : [["setnpctoken", "step", "mountains"]],
  "msg" : ["Actually I do. I followed him to a small cave in a small mountain range, just south of Darcap.\n\nBut I didn't dare enter, I've heard weird sounds, like undead howls, and felt some breath of big creatures.\n\n*shiver*"],
  "replies" : [["search", "Well, if you want, I can go and recover it.", "0"]]
  },{
  "match" : ["scythe"],
  "pre" : [["npctoken", "step", "greeted"]],
  "post" : [["setnpctoken", "step", "stolen"]],
  "msg" : ["Actually I didn't lose it, a gnoll stole it from me!"],
  "replies" : [["where", "Do you have any idea where this gnoll ran away?", "2"]]
  },{
  "match" : ["hello", "hi"],
  "pre" : [["quest", "darcap/Cameron-wine", "20"]],
  "post" : [["setnpctoken", "step", "greeted"]],
  "msg" : ["Hello.\n\nWhat can I do to help you?"],
  "replies" : [["scythe", "According to Scartis, you lost your scythe, and thus can't cut grapes?", "2"]]
  },{
  "match" : ["hello", "hi"],
  "pre" : [],
  "post" : [],
  "msg" : ["Hello.\n\nWhat can I do to help you?"],
  "replies" : []
  },{
  "match" : ["*"],
  "pre" : [],
  "post" : [["setnpctoken", "step", "start"]],
  "msg" : ["..."],
  "replies" : []
  }
]}
