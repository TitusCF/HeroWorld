{
  "location" : "scorn/alfalfa",
  "rules": [
  {
  "match" : ["gate", "pass", "merchant"],
  "pre" : [],
  "post" : [],
  "msg" : ["Just go to the farthest corner from the door and dig into the wall and you will find it."]
  },{
  "match" : ["guard"],
  "pre" : [],
  "post" : [],
  "msg" : ["They won't give me any clothes either. And I hear the post across the corridor has become a most prized job by the guards!"]
  },{
  "match" : ["yes"],
  "pre" : [["quest", "scorn/OrcDress", "10-30"],["item", "Alfalfa's dress"]],
  "post" : [["takeitem", "Alfalfa's dress"], ["quest", "scorn/OrcDress", "40"], ["givecontents", "alfalfaprize"]],
  "msg" : ["Thank you for getting me some clothes $you, I don't have anything I can give you here, but if you go back to my house, in the basement, there is a secret hiding place where I keep my valuables, underneath a rock in the south west corner."]
  },{
  "match" : ["yes"],
  "pre" : [["quest", "scorn/OrcDress", "10-30"]],
  "post" : [],
  "msg" : ["Where are they?"]
  },{
  "match" : ["no"],
  "pre" : [["quest", "scorn/OrcDress", "10-30"]],
  "post" : [],
  "msg" : ["Oh do hurry up, I don't like the prison guards around here."],
  "replies" : [["guards", "Why don't you like the prison guards?", 2]]
  },{
  "match" : ["no"],
  "pre" : [],
  "post" : [],
  "msg" : ["Oh you are just as mean as the prison guards."],
  "replies" : [["guards", "What's wrong with the prison guards?", 2]]
  },{
  "match" : ["clothes"],
  "pre" : [["quest", "scorn/OrcDress", "40"]],
  "post" : [],
  "msg" : ["Thank you for getting me some clothes $you"],
  "replies" : [["house", "Where is your house?", 2]]
  },{
  "match" : ["clothes"],
  "pre" : [],
  "post" : [],
  "msg" : ["Well, I doubt that you could find clothes that fit me, you will need to get some from my house"],
  "replies" : [["house", "Where is your house?", 2]]
  },{
  "match" : ["key"],
  "pre" : [],
  "post" : [["quest", "scorn/OrcDress", "10"]],
  "msg" : ["The door slammed shut behind me as I fled, locking me out. My friend Fran has a spare key to my house, ask her for it, she works over at the barking mule. Tell her I sent you.\n"]
  },{
  "match" : ["house"],
  "pre" : [],
  "post" : [],
  "msg" : ["I'm Alfalfa, and I live, or used to live at, Number 32, River Road.\n It's at the south side of the town, just a few chains south of the wanderer's inn. You'll need a key to get in\n"],
  "replies" : [["key", "Don't you have a key?", 2]]
  },{
  "match" : ["streaking", "arrest", "naked"],
  "pre" : [],
  "post" : [],
  "msg" : ["I had to run from my house in the middle of the night, as band of orcs crashed though my bedroom door!"],
  "replies" : [["house", "where is your house?", 2]]
  },{
  "match" : ["*"],
  "pre" : [["quest", "scorn/OrcDress", "40"]],
  "post" : [],
  "msg" : ["Thank you for getting clothes for me $you"]
  },{
  "match" : ["*"],
  "pre" : [["quest", "scorn/OrcDress", "10-30"]],
  "post" : [],
  "msg" : ["Hello, $you, have you been able to find some clothes for me"],
  "replies" : [["yes", "Yes, here you are"], ["no", "No, no joy yet"]]
  },{
  "match" : ["*"],
  "pre" : [["quest", "scorn/OrcDress", "=0"]],
  "post" : [],
  "msg" : ["I was arrested for streaking.\nWould you be able to get me some clothes please?"],
  "replies" : [["streaking", "Why were you streaking?", 2], ["clothes", "Where can I get clothes from?", 2], ["no", "I'm sorry, I don't have time to go fetching clothes"]]
  }
]}
