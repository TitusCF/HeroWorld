{
  "location" : "scorn/portguard",
  "rules": [
  {
  "match" : ["clerk", "Milton", "office", "mood"],
  "pre" : [["quest", "scorn/PassClerk", "60"]],
  "post" : [],
  "msg" : ["I don't know how you did it, but you seem to have put Milton in a good mood, I've never seen him like this before."]
  },{
  "match" : ["clerk", "Milton", "office", "mood"],
  "pre" : [["quest", "scorn/PassClerk", "50"]],
  "post" : [],
  "msg" : ["Thank you for getting Milton out of that foul mood he was in, maybe he'll be a bit more bearable now."]
  },{
  "match" : ["clerk", "Milton", "office", "mood"],
  "pre" : [["quest", "scorn/PassClerk", "10"]],
  "post" : [["quest", "scorn/PassClerk", "20"]],
  "msg" : ["Oh, you spoke to Milton then, he can be a bit intemperate at times, particularly when he hasn't had his coffee."],
  "replies" : [["coffee", "Does he drink a lot of coffee then?"]]
  },{
  "match" : ["clerk", "Milton", "office"],
  "pre" : [],
  "post" : [],
  "msg" : ["Milton is the clerk who works in the pass office, he should be able to help you."]
  },{
  "match" : ["coffee"],
  "pre" : [["quest", "scorn/PassClerk", "20"]],
  "post" : [],
  "msg" : ["I'd swear Milton was addicted to his coffee, he is a complete nightmare to deal with if he hasn't had some for a while."]
  },{
  "match" : ["chain"],
  "pre" : [["quest", "scorn/GatePassword", "30"]],
  "post" : [["connection", "14"]],
  "msg" : ["Oh you know the password! You must be a citizen of scorn.\n Pass Citizen."]
  },{
  "match" : ["chain"],
  "pre" : [],
  "post" : [["quest", "scorn/GatePassword", "30"], ["connection", "14"]],
  "msg" : ["Oh you know the password! You must be a citizen of scorn.\n Pass Citizen."]
  },{
  "match" : ["king"],
  "pre" : [["quest", "scorn/ScornHero", "10"]],
  "post" : [],
  "msg" : ["You'll find him in scorn castle, you'll probably need to speak to the magistrate rather than directly to the king though."]
  },{
  "match" : ["password"],
  "pre" : [["quest", "scorn/PortGate", "5"], ["quest", "scorn/GatePassword", "10"]],
  "post" : [],
  "msg" : ["I am not allow to tell you the password."]
  },{
  "match" : ["password"],
  "pre" : [["quest", "scorn/PortGate", "5"]],
  "post" : [["quest", "scorn/GatePassword", "10"]],
  "msg" : ["I am not allow to tell you the password."]
  },{
  "match" : ["hero"],
  "pre" : [["quest", "scorn/ScornHero", "40"]],
  "post" : [],
  "msg" : ["Yes, you are a hero of Scorn, $you. I salute you."]
  },{
  "match" : ["hero"],
  "pre" : [["quest", "scorn/PortGate", "5"], ["quest", "scorn/ScornHero", "10"]],
  "post" : [],
  "msg" : ["Only the king can grant `Hero' status."],
  "replies" : [["king", "Where is the King?"]]
  },{
  "match" : ["hero"],
  "pre" : [["quest", "scorn/PortGate", "5"]],
  "post" : [["quest", "scorn/ScornHero", "10"]],
  "msg" : ["Only the king can grant `Hero' status."],
  "replies" : [["king", "Where is the King?"]]
  },{
  "match" : ["port", "pass"],
  "pre" : [["quest", "scorn/PortGate", "5"],["quest", "scorn/PortPass", "10"]],
  "post" : [],
  "msg" : ["The Office of Gate Passes is just beyond the sign to the left, speak to the clerk and he'll help you, maybe."],
  "replies" : [["clerk", "Who is the Clerk?"]]
  },{
  "match" : ["port", "pass"],
  "pre" : [["quest", "scorn/PortGate", "5"]],
  "post" : [["quest", "scorn/PortPass", "10"]],
  "msg" : ["See the Office of Gate Passes over to the left."]
  },{
  "match" : ["who", "enter", "people"],
  "pre" : [["quest", "scorn/PortGate", "5"]],
  "post" : [["quest", "scorn/PortGate", "10"]],
  "msg" : ["Yes you can only pass if you:\n* Have a merchant's ``Port Pass''\n* Are a known ``Hero of Scorn''\n* or know todays password."],
  "replies" : [["pass", "Where can I get a 'Port Pass' from?"], ["hero", "How can I become a ``Hero of Scorn''?"], ["password", "What is the password?"]]
  },{
  "match" : ["*"],
  "pre" : [["quest", "scorn/PortGate", "5"]],
  "post" : [],
  "msg" : ["Only certain people may enter the port."],
  "replies" : [["people", "Who is permitted to enter?"]]
  },{
  "match" : ["*"],
  "pre" : [],
  "post" : [["quest", "scorn/PortGate", "5"]],
  "msg" : ["Only certain people may enter the port."],
  "replies" : [["people", "Who is permitted to enter?"]]
  }
]}
