{
  "location" : "newbie_sigmund",
  "rules": [
  {
  "comment" : "**** experience thread ****",
  "match" : ["yes", "sound fun", "sounds fun", "experience", "teach"],
  "pre" : [["token", "proposal", "experience"]],
  "post" : [["settoken", "proposal", "pathExplain"]],
  "msg" : ["Experience comes by adventuring in the world, and knowing what your level is. Don't try too difficult things and do concentrate on your profession and class. If you are a caster, don't just run and bash in the enemies; you are not solid enough. If you are a barbarian, don't try to cast icestorm at level 1, you are supposed to use your muscles! What are you?"]
  }, {
  "match" : ["bow", "range", "archer"],
  "pre" : [["token", "proposal", "pathExplain"]],
  "post" : [["settoken", "proposal", ""]],
  "msg" : ["Some people will probably call you a coward, but ranged weapons are by far one of the most difficult ones to master. You need to have plenty of arrows ready, and will need to replace them on a regular basis. If your arrows are magical, replacing them is expensive. If that isn't enough, note that it is not always easy to aim. You still can fight with melee weapons if you are surrounded, but you probably shouldn't carry around a lot of heavy equipment."]
  }, {
  "match" : ["fighter", "barbarian", "melee"],
  "pre" : [["token", "proposal", "pathExplain"]],
  "post" : [["settoken", "proposal", ""]],
  "msg" : ["Relying on your muscles? Don't overlook what magic can do for you.  You can wear heavy armor (unless you are a race without a torso? My eyes are not what they used to be). You can use shields, and can quickly fight your way through a pack of monsters. Be careful when surrounded though; you can't defend from all sides, and will suffer more. If you travel with a priest, they may help you heal more quickly than you could on your own."]
  }, {
  "match" : ["caster", "mage", "magic"],
  "pre" : [["token", "proposal", "pathExplain"]],
  "post" : [["settoken", "proposal", ""]],
  "msg" : ["Ah, the magic! Stay away from it! If you really want to know, magic is conjured from some special body energy, named \"mana\". that is converted to spell energy during casting. To know about such convertion you must be skilled in one of the various magical paths and will need to use books or scrolls. With books you can memorize spells, whereas scrolls just act as a catalyst for your energy, and are destroyed after use. I heard some magical paths are antagonists."]
  }, {
  "match" : ["priest"],
  "pre" : [["token", "proposal", "pathExplain"]],
  "post" : [["settoken", "proposal", "pray"]],
  "msg" : ["So your fate will lie in the hands of some never seen entity that helps you when it is in the right mood? If you embrace the career of a priest, you will need to devote yourself to a god. To do this, find the temple of the god you want to worship and pray on it's altar. To cast a priestly spell, you need to be on the good side of your god. The more you pray, the more you are in his good graces. If your grace becomes low, god may still fulfill your request, or he may just punish you for being so presumptuous!"]
  }, {
  "match" : ["pray"],
  "pre" : [["token", "proposal", "pray"]],
  "post" : [["settoken", "proposal", ""]],
  "msg" : ["As a priest, you have a special skill for praying. It allows you to stay in the good graces of your god. As you pray on altars, your god might give you temporary bonuses too, or even drop you some lost artifact, but this is very uncommon, and often only occurs later in one's career!"]
  }, {
  "match" : ["*"],
  "pre" : [["token", "proposal", "pathExplain"]],
  "post" : [],
  "msg" : ["What is your character class? Are you a caster? Are you more of a priest? A bowyer? A fighter!", "I sense you are a fighter or a priest! What is your class?", "That's not helping! Caster, priest, fighter, using range weapons? What is your profession?", "There is not much choice you can make at the start of the game.  Either you embraced a caster or priest path, or you embraced ranged weapons, or you engaged in the use of melee weapons", "Did I tell you already I once killed a kobold?"]
  }, {
  "comment" : "**** bindings object thread ****",
  "match" : ["yes", "bind"],
  "pre" : [["token", "proposal", "bindings"]],
  "post" : [["settoken", "proposal", "bindings2"]],
  "msg" : ["Binding is a way to associate a keyboard key with a Crossfire action.  Lots of actions are available in Crossfire. Use the 'help commands' to learn about them. To bind the command 'apply', type 'bind apply' then hit the key you want to use to apply things with. Various Crossfire clients provide a custom menu to easily manipulate bindings. There is also a set of default keybindings."]
  }, {
  "match" : ["default"],
  "pre" : [["token", "proposal", "bindings2"]],
  "post" : [["settoken", "proposal", ""]],
  "msg" : ["The default set of keybindings is available to all clients\n, picks things up\n' gives immediate access to command box,\nNumpad & arrows allows you to move,\nCtrl enables/disables run mode\nShift enables/disables firing mode (for throwing or spell casting).\ns may be used to search your surroundings.\nd is useful for disarming traps.\nThere are probably others too, but give this old man a break, eh?"]
  }, {
  "match" : ["custom", "client"],
  "pre" : [["token", "proposal", "bindings2"]],
  "post" : [],
  "msg" : ["Yes, clients can help you manipulate keybindings, making available combinations of the control, shift, meta or Apple keys. They do this using in specialized dialog boxes. The older GTK client does it using the Client -> Configure -> Keybindings menu, and the newer one uses Client -> Keybindings."]
  }, {
  "comment" : "**** apply object thread ****",
  "match" : ["yes", "apply", "more"],
  "pre" : [["token", "proposal", "apply"]],
  "post" : [["settoken", "proposal", "bindings"]],
  "msg" : ["Some objects in Crossfire react to 'Applying'. To apply an object (lever, sign, ...), stand above it and type 'apply'. This applies the object.  Of course, it might be quite cumbersome to do that again and again. The easiest way to apply is to use a binding in the client. Do you want to learn about that?"]
  }, {
  "comment" : "**** saving thread ****",
  "match" : ["yes", "saving"],
  "pre" : [["token", "proposal", "saving"]],
  "post" : [["settoken", "proposal", "apply"]],
  "msg" : ["You cannot use a bed to reality until you have earned at least one experience point. You must use a bed to reality at least once to have your character saved. You can find beds to reality just southwest of here. Do you want to get more information on how to apply an object in Crossfire."]
  }, {
  "comment" : "**** bed to reality thread ****",
  "match" : ["yes", "more"],
  "pre" : [["token", "proposal", "bed"]],
  "post" : [["settoken", "proposal", "bed2"]],
  "msg" : ["A bed to reality is the way you quit the Crossfire world to get back to your real life. While you are in real life, your character is asleep.  Nothing can happen to it, but beware of losing your items! Want to know more?"]
  }, {
  "match" : ["yes", "more"],
  "pre" : [["token", "proposal", "bed2"]],
  "post" : [["settoken", "proposal", "saving"]],
  "msg" : ["All items left on the ground while you are in real life may disappear or be stolen by other players. If you want to keep them with you, they must be put into your inventory or in a permanent apartment. Do you want me to tell you about saving your character?"]
  }, {
  "comment" : "**** food thread ****",
  "match" : ["yes", "food", "eat"],
  "pre" : [["token", "proposal", "food"]],
  "post" : [["settoken", "proposal", ""]],
  "msg" : ["There are plenty of snacks in the wild world (and in shops). Before going to adventure, always bring a bag of healthy food with you. Don't die starving. If you forget to eat, you might become a bit crazy and eat whatever happens to be in your pack. Beware of poisoned food my lad! I think i saw a safe snack nearby :)"]
  }, {
  "comment" : "**** healing thread ****",
  "match" : ["yes", "heal"],
  "pre" : [["token", "proposal", "heal"]],
  "post" : [["settoken", "proposal", "heal2"]],
  "msg" : ["You have hit points in Crossfire. If you don't heal after losing life points, you might die. Your character heals naturally, but you can also use potions or spells to heal faster. Praying on your god's altar might also prove useful. Do you want me to instruct you on the dangers in the Crossfire world? Or, maybe, on natural healing?"]
  }, {
  "match" : ["danger", "damage"],
  "pre" : [["token", "proposal", "heal2"]],
  "post" : [["settoken", "proposal", "food"]],
  "msg" : ["Most wild monsters deal damage. Some of them deal damage faster than other ones. Traps and spells can also deal damage. Even some food can damage as there is there poisoned food to be found in Crossfirer, but you *need* to eat! Wanna get some info on eating?"]
  }, {
  "match" : ["natural", "heal"],
  "pre" : [["token", "proposal", "heal2"]],
  "post" : [["settoken", "proposal", "food"]],
  "msg" : ["Your wounds close and heal naturally in Crossfire. If you are taking damage though, it is necessary to eat more often.  As one might say, \"to stay healthy, eat often\". Or maybe that was \"an orc chop a day, keeps the doctor away\"? Anyway, there is a wide variety of snacks for adventurers, but be careful. Want me to instruct you on food?"]
  }, {
  "match" : ["yes"],
  "pre" : [["token", "proposal", "heal2"]],
  "post" : [],
  "msg" : ["Yes what?", "Yes I want to know about natural healing? Or yes I want to know about dangerous areas? Are you sure you are listening to me?", "I once killed a kobold, you know?"]
  }, {
  "comment" : " ***** Generic subjects player might want to abort. Switch to proposal *****",
  "match" : ["bed", "reality"],
  "pre" : [["token", "dialog", "started"]],
  "post" : [["settoken", "proposal", "bed"]],
  "msg" : ["A bed to reality is the way you quit the Crossfire world. Want to know about it?"]
  }, {
  "match" : ["experience"],
  "pre" : [["token", "dialog", "started"]],
  "post" : [["settoken", "proposal", "experience"]],
  "msg" : ["Experience? I *am* a man of experience! I have traveled the world! And if you want, just between you and me, I can teach you about experience. Sound fun?"]
  }, {
  "match" : ["bind"],
  "pre" : [["token", "dialog", "started"]],
  "post" : [["settoken", "proposal", "bindings"]],
  "msg" : ["Bindings links your keyboard to specific Crossfire commands. Want to know about it?"]
  }, {
  "match" : ["apply", "use", "item"],
  "pre" : [["token", "dialog", "started"]],
  "post" : [["settoken", "proposal", "apply"]],
  "msg" : ["You need to learn how to use items if you want to interact with the Crossfire world. Want to learn how to do that?"]
  }, {
  "match" : ["heal", "wound"],
  "pre" : [["token", "dialog", "started"]],
  "post" : [["settoken", "proposal", "heal"]],
  "msg" : ["Healing is important, if you don't heal, D.E.A.T.H will get you! Want to know about it?"]
  }, {
  "match" : ["no"],
  "pre" : [["token", "dialog", "started"]],
  "post" : [["settoken", "proposal", ""]],
  "msg" : ["That's ok pal, want something more?", "So you think you know everything?\nThose children!", "Adventurers, all the same, no time\n to listen to old, wise Sigmund"]
  }, {
  "comment" : "******  Various non threaded dialogs / basic proposals *****",
  "match" : ["spell", "cast"],
  "pre" : [["token", "dialog", "started"]],
  "post" : [["settoken", "proposal", ""]],
  "msg" : ["I am a old wise man, and as the old wise man I am, I give you this advice.\nStay away from magic!\nIt burns your soul, kills your teammates and, if you use the magic of the gods, it can destroy you. Wild beasts sometimes use magical powers, be careful and if possible use amulets or rings to protect you. My grand grand cousin has a magic shop, \"The Lord of The Rings\", it full of usefull stuff. A bit expensive though!"]
  }, {
  "match" : ["*"],
  "pre" : [["token", "dialog", "0"]],
  "post" : [["settoken", "dialog", "started"]],
  "msg" : ["Hello, dear newbie. I am an NPC. This means you can interact with me. To do so, just speak to me. Like all NPCs in Crossfire, I react to various words you say when you are near me (using the 'say <message>' command). Just say you need help, and I'll be glad to help you!"]
  }, {
  "match" : ["*"],
  "pre" : [["token", "dialog", "started"]],
  "post" : [["settoken", "proposal", "bed"]],
  "msg" : ["I know everything about what you should do next: where to heal, how to bind keys, how to use items, beds to reality, and gain experience. Would you like me to tell you about beds to reality? That's important in Crossfire."]
  }
]}
