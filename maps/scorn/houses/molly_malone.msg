{
  "location" : "scorn_malone",
  "rules": [
  {
  "match" : ["hard", "hearing", "deaf", "ear", "horn"],
  "pre" : [["quest", "scorn/Earhorn", "60"]],
  "post": [],
  "msg" : ["Thank you for returning my father's earhorn $you."]
  },{
  "match" : ["hard", "hearing", "deaf", "ear", "horn"],
  "pre" : [["quest", "scorn/Earhorn", "20"]],
  "post": [],
  "msg" : ["Good luck tracking down my father's horn"]
  },{
  "match" : ["hard", "hearing", "deaf"],
  "pre" : [],
  "post": [["quest", "scorn/Earhorn", "20"]],
  "msg" : ["He's been using an earhorn for the last couple of years so he can hear what I tell him, not that he pays much attention, but he managed to lose it recently, so now he can't even hear to ignore me.\n If you should happen to come across his earhorn in your travels, please bring it back to him."]
  },{
  "match" : ["forgetful", "stick", "staff", "walking"],
  "pre" : [["quest", "scorn/WalkingStick", "60"]],
  "post": [],
  "msg" : ["Thank you for returning my father's walking stick, $you."]
  },{
  "match" : ["forgetful", "stick", "staff", "walking"],
  "pre" : [["quest", "scorn/WalkingStick", "20"]],
  "post": [],
  "msg" : ["Good luck finding my father's walking stick."]
  },{
  "match" : ["forgetful"],
  "pre" : [],
  "post": [["quest", "scorn/WalkingStick", "20"]],
  "msg" : ["Why yes, just the other day he lost his walking stick of all things. \nI can't for the life of me figure out how he got home without it.\n You couldn't keep a lookout for it for him could you? He's been stuck in the house ever since he lost it"]
  },{
  "match" : ["work", "job"],
  "pre" : [],
  "post": [],
  "msg" : ["I used to be a fishmonger."]
  },{
  "match" : ["dublin", "fair city"],
  "pre" : [],
  "post": [],
  "msg" : ["I'm sorry, I don't know where that is, I used to work around darcap before I gave it up to look after my father"]
  },{
  "match" : ["cockles", "mussels", "barrow"],
  "pre" : [],
  "post": [],
  "msg" : ["I had to give up the fishmongering to look after my father"]
  },{
  "match" : ["*"],
  "pre" : [],
  "post": [["settoken", "seenbefore", "yes"]],
  "msg" : ["Hello, I'm Molly, I live here in cornerbrook with my father, he's been getting a bit forgetful and rather hard of hearing, so I've been looking after him."]
  }
]}