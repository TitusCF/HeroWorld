{
  "location" : "scorn/eswald",
  "rules": [
  {
  "match" : ["yes", "like"],
  "pre" : [["token", "offercoffee", "yes"], ["item", "money", "1"], ["quest", "scorn/CoffeeGoose", "100"]],
  "post" : [["giveitem", "Eswald's superior coffee"], ["settoken", "offercoffee", "0"], ["takeitem", "money", "1"]],
  "msg" : ["Here you go, a cup of the best coffee you'll ever drink."]
  }, {
  "match" : ["yes", "like"],
  "pre" : [["token", "offercoffee", "yes"], ["quest", "scorn/CoffeeGoose", "100"]],
  "post" : [["settoken", "offercoffee", "0"]],
  "msg" : ["Sorry $you, but 1 silver is the lowest I can do, even for you..."]
  }, {
  "match" : ["no"],
  "pre" : [["quest", "scorn/CoffeeGoose", "100"], ["token", "offercoffee", "yes"]],
  "post" : [["settoken", "offercoffee", "0"]],
  "msg" : ["Ok, come back if you change your mind."]
  }, {
  "match" : ["*"],
  "pre" : [["quest", "scorn/CoffeeGoose", "100"]],
  "post" : [["settoken", "offercoffee", "yes"]],
  "msg" : ["Welcome back to to Eswald's coffee shop, $you, would you like some of the finest brew east of Navar? Only 1 silver to you, because of all you did for me!"]
  }, {
  "match" : ["no"],
  "pre" : [["quest", "scorn/CoffeeGoose", "20"], ["token", "askgoose", "yes"]],
  "post" : [["settoken", "askgoose", "0"]],
  "msg" : ["Well, keep looking, "]
  }, {
  "match" : ["yes", "found"],
  "pre" : [["quest", "scorn/CoffeeGoose", "20"], ["token", "askgoose", "yes"], ["item", "EsmeraldaTheGoose"]],
  "post" : [["settoken", "askgoose", "0"], ["giveitem", "money", "5000"], ["quest", "scorn/CoffeeGoose", "100"], ["takeitem", "EsmeraldaTheGoose"]],
  "msg" : ["You found her!  Here is your reward.  Thanks $you.\n\n Esmeralda has the finest nose...er... beak for coffee in the kingdom.\n Oled will be so jealous of my coffee."]
  }, {
  "match" : ["yes", "found"],
  "pre" : [["quest", "scorn/CoffeeGoose", "20"], ["token", "askgoose", "yes"]],
  "post" : [["settoken", "askgoose", "0"]],
  "msg" : ["You have, that's fantastic! bring her back here and I'll pay you your reward."]
  }, {
  "match" : ["*"],
  "pre" : [["quest", "scorn/CoffeeGoose", "20"]],
  "post" : [["settoken", "askgoose", "yes"]],
  "msg" : ["Have you found Esmerelda yet?"]
  }, {
  "match" : ["clue", "idea", "lead", "lost", "suspicious"],
  "pre" : [["quest", "scorn/CoffeeGoose", "20"]],
  "post" : [],
  "msg" : ["Well Oled has always been jealous of my goose..."]
  }, {
  "match" : ["special"],
  "pre" : [["quest", "scorn/CoffeeGoose", "20"]],
  "post" : [],
  "msg" : ["I'm not telling, you might not bring her back...."]
  }, {
  "match" : ["*"],
  "pre" : [["quest", "scorn/CoffeeGoose", "20"]],
  "post" : [],
  "msg" : ["Have you had any luck finding my goose?"]
  }, {
  "match" : ["yes", "interested", "goose"],
  "pre" : [["quest", "scorn/CoffeeGoose", "10"]],
  "post" : [["quest", "scorn/CoffeeGoose", "20"]],
  "msg" : ["Why, thank you, Her name is Esmeralda, she's a special goose."]
  }, {
  "match" : ["*"],
  "pre" : [["quest", "scorn/CoffeeGoose", "10"]],
  "post" : [],
  "msg" : ["Yes, Yes, I know, my coffee is terrible, I'm truely very sorry, it just hasn't been the same since I lost my goose. \n\n You wouldn't be interested in finding her for me would you?"]
  }, {
  "match" : ["*"],
  "pre" : [["token", "scorn/CoffeeGoose", "10"]],
  "post" : [],
  "msg" : ["Yes, Yes, I know, my coffee is terrible, I'm truely very sorry, it just hasn't been the same since I lost my goose. \n\n You wouldn't be interested in finding her for me would you?"]
  }, {
  "match" : ["no"],
  "pre" : [["token", "offercoffee", "yes"]],
  "post" : [["settoken", "offercoffee", "0"], ["settoken", "badcoffee", "yes"]],
  "msg" : ["I'm not surprised, no one else does either nowadays"]
  }, {
  "match" : ["yes", "like"],
  "pre" : [["token", "offercoffee", "yes"], ["item", "money", "2"]],
  "post" : [["giveitem", "Eswald's inferior coffee"], ["settoken", "offercoffee", "0"], ["settoken", "badcoffee", "yes"], ["takeitem", "money", "2"]],
  "msg" : ["Here you go, I apologise for the quality"]
  }, {
  "match" : ["yes", "like"],
  "pre" : [["token", "offercoffee", "yes"], ["token", "freebie", "yes"]],
  "post" : [["settoken", "offercoffee", "no"], ["settoken", "badcoffee", "yes"], ["takeitem", "money", "2"]],
  "msg" : ["You'll need to pay for coffee, friend, I may not have many customers, but I can't give it away free you know."]
  }, {
  "match" : ["yes", "like"],
  "pre" : [["token", "offercoffee", "yes"]],
  "post" : [["giveitem", "Eswald's inferior coffee"], ["settoken", "offercoffee", "no"], ["settoken", "badcoffee", "yes"], ["takeitem", "money", "2"], ["settoken", "freebie", "yes"]],
  "msg" : ["You haven't even got two silver coins to rub together? Here, have the coffee anyway, you clearly need a hand-out, and no one is buying it nowadays anyway"]
  }, {
  "match" : ["coffee"],
  "pre" : [],
  "post" : [["settoken", "offercoffee", "yes"]],
  "msg" : ["Would you like to buy some coffee, only 2 silver a cup?"]
  }, {
  "match" : ["hello", "hi"],
  "pre" : [["token", "seenbefore", "yes"]],
  "post" : [],
  "msg" : ["Hello $you, would you like to buy some coffee?"]
  }, {
  "match" : ["hello", "hi"],
  "pre" : [],
  "post" : [["settoken", "seenbefore", "yes"]],
  "msg" : ["Hello stranger. would you like to buy some coffee?"]
  }, {
  "match" : ["*"],
  "pre" : [],
  "post" : [["settoken", "seenbefore", "yes"]],
  "msg" : ["Welcome to Eswald's coffee shop, serving, well, something vaguely coffee-like anyway."]
  }
]}
