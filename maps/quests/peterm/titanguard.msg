{
  "location" : "wilderness/Titanguard",
  "rules": [
  {
  "match" : ["no"],
  "pre" : [["token", "question", "helptitan"]],
  "post" : [["settoken", "question", "0"]],
  "msg" : ["NONE SHALL pass..."]
  }, {
  "match" : ["yes", "booze"],
  "pre" : [["quest", "scorn/TitanCrown", "40"], ["token", "status", "askbooze"], ["item", "booze", "10"]],
  "post" : [["settoken", "status", "0"], ["quest", "scorn/TitanCrown", "50"], ["takeitem", "booze", "0"], ["connection", "1"]],
  "msg" : ["Aah, that's better....\n Well, I promised I'd let you past, and so I shall, just keep it a secret between ourselves, hmm?\n I'd get in trouble if it was discovered I was letting people through the gate."]
  }, {
  "match" : ["yes", "booze"],
  "pre" : [["quest", "scorn/TitanCrown", "40"], ["token", "status", "askbooze"], ["item", "booze"]],
  "post" : [["settoken", "status", "0"]],
  "msg" : ["Is that all you have? I'll need more than that to slake my thirst, I *am* a giant you know."]
  }, {
  "match" : ["yes", "booze"],
  "pre" : [["quest", "scorn/TitanCrown", "40"], ["token", "status", "askbooze"]],
  "post" : [["settoken", "status", "0"]],
  "msg" : ["Where is it?"]
  }, {
  "match" : ["thirsty"],
  "pre" : [["quest", "scorn/TitanCrown", "10-30"], ["token", "status", "wantsdrink"]],
  "post" : [["settoken", "status", "0"], ["quest", "scorn/TitanCrown", "40"]],
  "msg" : ["It's so hot today!  And we can't leave our posts.\n Say, if you brought some booze for me I'd be mighty obliged.  I am VERY thirsty!\n Yes, if you bring enough, I'll let you in!"]
  }, {
  "match" : ["*"],
  "pre" : [["quest", "scorn/TitanCrown", "40"]],
  "post" : [["settoken", "status", "askbooze"]],
  "msg" : ["Have you brought booze for me?"],
  "replies" : [["yes", "Yes, Here is your booze"], ["no", "No, I'm still looking for some booze for you"]]
  }, {
  "match" : ["*"],
  "pre" : [["quest", "scorn/TitanCrown", "10"]],
  "post" : [["settoken", "status", "wantsdrink"]],
  "msg" : ["NONE SHALL pass...\n\n Haaa, how thirsty am I...\n*sigh*"],
  "replies" : [["thirsty", "Would you like something to drink?"]]
  }, {
  "match" : ["*"],
  "pre" : [],
  "post" : [],
  "msg" : ["NONE SHALL pass..."]
  }
]}
