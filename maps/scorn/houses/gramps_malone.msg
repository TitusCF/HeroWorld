{
  "location" : "scorn_malone",
  "rules": [
  {
  "match" : ["stick", "staff", "walking"],
  "pre" : [["quest", "scorn/WalkingStick", "60"]],
  "post" : [],
  "msg" : ["Thank you for returning my walking stick, $you"]
  }, {
  "match" : ["stick"],
  "pre" : [],
  "post" : [],
  "msg" : ["Ya say yer sick?  That's a darn shame!\nI get a touch of the wind myself sometimes..."]
  }, {
  "match" : ["yes"],
  "pre" : [["quest", "scorn/Earhorn", "10"], ["token", "askfor", "horn"], ["item", "Gramps' earhorn"]],
  "post" : [["settoken", "askfor", "0"], ["takeitem", "Gramps' earhorn"], ["giveitem", "diamond", "3"], ["quest", "scorn/Earhorn", "60"]],
  "msg" : ["My horn!  Thanks.\n Here's a little spending money for your trouble.\n Now I can hear again.\n....err drat now she'll be gabbin at me all day.  Maybe I better hide it again..."]
  }, {
  "match" : ["yes"],
  "pre" : [["quest", "scorn/Earhorn", "10"], ["token", "askfor", "horn"]],
  "post" : [["settoken", "askfor", "0"]],
  "msg" : ["Well, where is it? I can't stand around here all day listening to you making excuses.\n Can't do much listening at all til you get me that horn."]
  }, {
  "match" : ["no"],
  "pre" : [["quest", "scorn/Earhorn", "10"], ["token", "askfor", "horn"]],
  "post" : [["settoken", "askfor", "0"]],
  "msg" : ["Go? Go where? I ain't going anywhere 'til you get me that horn."]
  }, {
  "match" : ["yes"],
  "pre" : [["quest", "scorn/WalkingStick", "10"], ["token", "askfor", "stick"], ["item", "Gramp's walking stick"]],
  "post" : [["settoken", "askfor", "0"], ["takeitem", "Gramp's walking stick"], ["giveitem", "diamond", "2"], ["quest", "scorn/WalkingStick", "60"]],
  "msg" : ["You found my staff, thanks.\nHeres a little something for you.\nDon't spend it all on sweets now."]
  }, {
  "match" : ["yes"],
  "pre" : [["quest", "scorn/WalkingStick", "10"], ["token", "askfor", "stick"]],
  "post" : [["settoken", "askfor", "0"]],
  "msg" : ["Well, where is it? I can't stand around here all day waiting for you to get your act together.\n 'course if you don't get me my stick back, I may not have a lot of choice."]
  }, {
  "match" : ["no"],
  "pre" : [["quest", "scorn/WalkingStick", "10"], ["token", "askfor", "stick"]],
  "post" : [["settoken", "askfor", "0"]],
  "msg" : ["Well, you'd better go look for it then hadn't you? I be standing round here waiting for you to get moving\nTsk, kids these days!"]
  }, {
  "match" : ["staff", "walking"],
  "pre" : [["quest", "scorn/WalkingStick", "20"]],
  "post" : [["settoken", "askfor", "stick"]],
  "msg" : ["Have you found my walking stick, young'un?"]
  }, {
  "match" : ["staff", "walking"],
  "pre" : [["quest", "scorn/WalkingStick", "10"]],
  "post" : [["settoken", "askfor", "stick"]],
  "msg" : ["Yup lost my walking stick - you haven't seen it lying around have you?"]
  }, {
  "match" : ["staff", "walking"],
  "pre" : [],
  "post" : [["quest", "scorn/WalkingStick", "30"]],
  "msg" : ["Yup lost my walking stick - have ya seen it?\n Bring it back here if you find it, y'hear."]
  }, {
  "match" : ["woods", "east"],
  "pre" : [["quest", "scorn/Earhorn", "60"]],
  "post" : [],
  "msg" : ["Yes, I used to go hunting out east, there used to more deer around then than there are nowadays"]
  }, {
  "match" : ["woods", "east"],
  "pre" : [],
  "post" : [],
  "msg" : ["Don't mumble - out with it now..."]
  }, {
  "match" : ["ear", "horn"],
  "pre" : [["quest", "scorn/Earhorn", "60"]],
  "post" : [],
  "msg" : ["Thank you for returning my earhorn, $you"]
  }, {
  "match" : ["horn"],
  "pre" : [["quest", "scorn/Earhorn", "20"]],
  "post" : [["settoken", "askfor", "horn"]],
  "msg" : ["Have you found my earhorn, young'un?"]
  }, {
  "match" : ["horn"],
  "pre" : [["quest", "scorn/Earhorn", "10"]],
  "post" : [["settoken", "askfor", "horn"]],
  "msg" : ["Yup lost my earhorn - you haven't seen it lying around have you?"]
  }, {
  "match" : ["horn"],
  "pre" : [],
  "post" : [["quest", "scorn/Earhorn", "30"]],
  "msg" : ["Yup lost my Earhorn - have ya seen it?\n Bring it back here if you find it, y'hear."]
  }, {
  "match" : ["ear"],
  "pre" : [],
  "post" : [],
  "msg" : ["You looking for deer?\nUsed to be a woodsman myself.\nTry the woods to the east - lots of game there..."]
  }, {
  "match" : ["*"],
  "pre" : [["quest", "scorn/Earhorn", "60"], ["quest", "scorn/WalkingStick", "60"]],
  "post" : [],
  "msg" : ["Thank you for returning my earhorn and walking stick $you"]
  }, {
  "match" : ["*"],
  "pre" : [["quest", "scorn/Earhorn", "10"], ["quest", "scorn/WalkingStick", "60"]],
  "post" : [],
  "msg" : ["I can walk again thanks to you, $you. now if only I had my horn back."]
  }, {
  "match" : ["*"],
  "pre" : [["quest", "scorn/Earhorn", "60"], ["quest", "scorn/WalkingStick", "10"]],
  "post" : [],
  "msg" : ["I can hear again thanks to you, $you. now if only I had my staff back."]
  }, {
  "match" : ["*"],
  "pre" : [["quest", "scorn/WalkingStick", "60"]],
  "post" : [["quest", "scorn/Earhorn", "10"]],
  "msg" : ["Thank you for returning my staff, you haven't seen my earhorn lying about anywhere have you?\n If you do find it, please tell me"]
  }, {
  "match" : ["*"],
  "pre" : [["quest", "scorn/Earhorn", "60"]],
  "post" : [["quest", "scorn/WalkingStick", "10"]],
  "msg" : ["Thank you for returning my earhorn, you haven't seen my staff lying about anywhere have you?\n If you do find it, please bring it back."]
  }, {
  "match" : ["*"],
  "pre" : [["quest", "scorn/Earhorn", "30"], ["quest", "scorn/WalkingStick", "30"]],
  "post" : [],
  "msg" : ["Did you get anywhere with finding either my horn or my staff?"]
  }, {
  "match" : ["*"],
  "pre" : [["quest", "scorn/WalkingStick", "30"]],
  "post" : [["settoken", "askfor", "stick"]],
  "msg" : ["Did you find my staff?"]
  }, {
  "match" : ["*"],
  "pre" : [["quest", "scorn/Earhorn", "30"]],
  "post" : [["settoken", "askfor", "horn"]],
  "msg" : ["Did you find my Earhorn"]
  }, {
  "match" : ["speak"],
  "pre" : [],
  "post" : [],
  "msg" : ["Bleak? Here? How dare you, you young scallywag, I don't go around to your home and call it bleak. You young'uns are all the same, judging people by the value of their possessions."]
  }, {
  "match" : ["*"],
  "pre" : [],
  "post" : [],
  "msg" : ["Hunn?  Speak up there!"]
  }
]}
