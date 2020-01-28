{
  "rules": [
  {
  "match" : ["password"],
  "pre" : [["quest", "scorn/CityGate", "5"], ["quest", "scorn/GatePassword", "10"]],
  "post" : [],
  "msg" : ["I am not allowed to tell you the password."]
  }, {
  "match" : ["password"],
  "pre" : [["quest", "scorn/CityGate", "5"]],
  "post" : [["quest", "scorn/GatePassword", "10"]],
  "msg" : ["I am not allowed to tell you the password."]
  }, {
  "match" : ["hero", "king"],
  "pre" : [["quest", "scorn/CityGate", "5"], ["quest", "scorn/ScornHero", "10"]],
  "post" : [],
  "msg" : ["Try asking in Scorn castle to find a way to win the King's favour."]
  }, {
  "match" : ["hero", "king"],
  "pre" : [["quest", "scorn/CityGate", "5"]],
  "post" : [["quest", "scorn/ScornHero", "10"]],
  "msg" : ["Try asking in Scorn castle to find a way to win the King's favour."]
  }, {
  "match" : ["gate", "pass"],
  "pre" : [["quest", "scorn/CityGate", "5"], ["quest", "scorn/GatePass", "10"]],
  "post" : [],
  "msg" : ["See the ``Office of Gate Passes'' next to the Port Gate."]
  }, {
  "match" : ["gate", "pass"],
  "pre" : [["quest", "scorn/CityGate", "5"]],
  "post" : [["quest", "scorn/GatePass", "10"]],
  "msg" : ["See the ``Office of Gate Passes''next to the Port Gate."]
  }, {
  "match" : ["who", "pass", "people"],
  "pre" : [["quest", "scorn/CityGate", "5"]],
  "post" : [["quest", "scorn/CityGate", "10"]],
  "msg" : ["Only those who are:\n * Citizens of Scorn with a Password\n* Have a merchants ``Gate Pass''\n* Or knighted ``Hero'' by the king.\nmay pass through these gates."],
  "replies" : [["pass", "Where can I get a 'Gate Pass' from?", "0"], ["hero", "How can I become a ``Hero of Scorn''?", "0"], ["password", "What is the password?", "0"]]
  }, {
  "match" : ["open", "gate"],
  "pre" : [["quest", "scorn/CityGate", "5"]],
  "post" : [],
  "msg" : ["The gate opens only for certain people."]
  }, {
  "match" : ["open", "gate"],
  "pre" : [],
  "post" : [["quest", "scorn/CityGate", "5"]],
  "msg" : ["The gate opens only for certain people."],
  "replies" : [["people", "For whom is the gate opened?"]]
  }, {
  "match" : ["secret", "wall", "door"],
  "pre" : [],
  "post" : [],
  "msg" : ["I can't tell you that! Do you want to be thrown in JAIL?"]
  }, {
  "match" : ["monster"],
  "pre" : [],
  "post" : [],
  "msg" : ["They keep trying to sneak in, but we always kill them."]
  }, {
  "match" : ["*"],
  "pre" : [],
  "post" : [],
  "msg" : ["Good to see a friend!\n Have you seen any monsters lately?"],
  "replies" : [["monsters", "Why are you having problems with monsters?", "2"]]
  }
]}
