{
  "location" : "scorn_goths",
  "rules": [
  {
  "match" : ["mark"],
  "pre" : [],
  "post": [],
  "msg" : ["Ya that's me.  Bugger off, I'm on a roll here.\nO 47"]
  }, {
  "match" : ["amy"],
  "pre" : [],
  "post": [],
  "msg" : ["Ya my old lady, she's kind of mad at me right now...\nWhen I hit it big she'll sing a different tune though."]
  },{
  "match" : ["goose", "esmeralda"],
  "pre" : [["quest", "scorn/CoffeeGoose", "40"]],
  "post": [],
  "msg" : ["Ok, ok, don't hurt me.  I took her.  Don't turn me in.  If you did they'd take the goose for evidence, and then you wouldn't get the reward.\nI hid the that bird at Amy's place."]
  },{
  "match" : ["goose", "esmeralda"],
  "pre" : [["quest", "scorn/CoffeeGoose", "20"]],
  "post": [["quest", "scorn/CoffeeGoose", "40"]],
  "msg" : ["Ok, ok, don't hurt me.  I took her.  Don't turn me in.  If you did they'd take the goose for evidence, and then you wouldn't get the reward.\nI hid the that bird at Amy's place."]
  },{
  "match" : ["key", "door"],
  "pre" : [["quest", "scorn/CoffeeGoose", "50"]],
  "post": [],
  "msg" : ["Ok, ok, the key's hidden at Swabbie's house.\nTell Chancy 'mumblypeg' and he'll fetch it for you."]
  },{
  "match" : ["key", "door"],
  "pre" : [["quest", "scorn/CoffeeGoose", "40"]],
  "post": [["quest", "scorn/CoffeeGoose", "47"]],
  "msg" : ["Ok, ok, the key's hidden at Swabbie's house.\nTell Chancy 'mumblypeg' and he'll fetch it for you."]
  },{
  "match" : ["name"],
  "pre" : [],
  "post": [],
  "msg" : ["(mumble) ...tell my... name... (grumble)  Scat!"]
  },{
  "match" : ["job"],
  "pre" : [],
  "post": [],
  "msg" : ["Look, mate... Isn't it obvious?"]
  },{
  "match" : ["*"],
  "pre" : [],
  "post": [],
  "msg" : ["Go away, I'm busy.\nN 15"]
  }
]}