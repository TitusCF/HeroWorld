{
  "location" : "scorn_riverside",
  "rules": [
  {
  "match" : ["mark"],
  "pre" : [],
  "post": [],
  "msg" : ["That bum, all this time he had a girl on the side.\nToo bad he was coming into some coin too...\nSome deal with that bird of his."]
  }, {
  "match" : ["oled"],
  "pre" : [],
  "post": [],
  "msg" : ["He's ok to work for, but he's a pincher if you know what I mean.\nHe's usually pretty mellow, but lately he seems a bit tense."]
  },{
  "match" : ["pincher"],
  "pre" : [],
  "post": [],
  "msg" : ["You try it an I'll deck you I says to him..."]
  },{
  "match" : ["bird", "goose"],
  "pre" : [["quest", "scorn/CoffeeGoose", "30"]],
  "post": [],
  "msg" : ["Ya I heard Mark stole a goose."]
  },{
  "match" : ["bird", "goose"],
  "pre" : [["quest", "scorn/CoffeeGoose", "20"]],
  "post": [["quest", "scorn/CoffeeGoose", "30"]],
  "msg" : ["Ya I heard Mark stole a goose."]
  },{
  "match" : ["bird", "goose"],
  "pre" : [],
  "post": [],
  "msg" : ["Ya I heard Mark stole a goose."]
  },{
  "match" : ["who"],
  "pre" : [],
  "post": [],
  "msg" : ["Your pretty nosey there fella."]
  },{
  "match" : ["service", "order"],
  "pre" : [],
  "post": [],
  "msg" : ["Hey buddy just take a seat, I work two jobs you know."]
  },{
  "match" : ["job"],
  "pre" : [],
  "post": [],
  "msg" : ["Ya I work part time at Oled's Cafe too."]
  },{
  "match" : ["alfalfa"],
  "pre" : [["quest", "scorn/OrcDress", "=10"]],
  "post": [["giveitem", "Alfalfa's Front Door Key"],["quest", "scorn/OrcDress", "20"]],
  "msg" : ["Alfalfa sent you to get the key to her house? Ok then, here you go."]
  },{
  "match" : ["*"],
  "pre" : [],
  "post": [],
  "msg" : ["Hey kitchen is off limits!\nKeep it in your scabbard and take a seat there chum."]
  }
]}
