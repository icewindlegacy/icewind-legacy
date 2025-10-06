``` 
          .mm              '        ]     .                       
           ]   m,  m, ,  ,.m  ..,  md     ]    m,  mm  m,  m, . . 
           ]  ]'' ]'] \..' ]  ]'] ]'T     ]   ]'] ]'T ' ] ]'' ',/ 
           ]  ]   ]"" ]dd  ]  ] ] ] ]     ]   ]"" ] ] ."T ]    b[ 
          .dm 'b/ 'b/  [[ .dm ] ] 'bW     ]mm,'b/ 'bT 'mT 'b/  T  
                                                  ,]          / 
                                                 ''         '' 
```
                                  A   
# MARS - Multi Adventurer Roleplaying System Game
  
MARS is a ROM-based MUD codebase, with lots of changes. Many of them are to bring
the game in line with the d20 SRD (3.5e) as released under the Open Gaming License
with a world that is ready to be created and populated by the DMs. The skills and 
spells will all be based on feats and spells from the SRD, as are the races and 
classes. The majority of the game is already in line with the SRD, but there are
still some things that need to be added/changed so it is a work in progress.   
  
However, I felt that the best way to actually *see* that progress would be to open
MARS up to the community as a community project. I've been working on it since 2015
mostly by myself (Though parts of the code such as mounts, vehicles and overland are
from a previous project and were not written by me), starting with ROM2.4b6 as a base
and then adding/subtracting/changing code as I went. Very little of the MUD is from
snippets (as many of them would need to be modified to work with MARS anyway) though
on occassion I would see a snippet, like the overall idea, but implement it in my own
way.  
  
At any rate, MARS will take some work on your end to compile on your system, as it has
a few requirements that most MUDs do not (SQL, ImageMagick, to name a couple) but I do
happen to know of a MUD hosting service where it will compile pretty easily;  
  
https://vineyard.haus 
  
They're my host of choice, and where I run my personal copy of MARS.  
  
> TO COMPILE & RUN  
>===========  
 depends: libmysql libgd  
As of last build, mysql.h has begun throwing errors so I replaced it with the mysql.h  
from mariadb on my machine, your mileage may vary so I left it as mysql/mysql.h for  
the time being.  
  
 cd to the src directory    
   
 type 'make'    
   
 when the MUD is finished compiling, type "nophup ./startup &"    

 The MUD will be running on port 3033.   
     
If you need to run it on a different port then you would type   
"nohup ./startup <port number> &" replacing <port number> with the port number    
 you prefer to use. (do not type the <> symbols.)  


Telnet or use a MUD client to connect to your host.  
If you're running it locally, it's localhost on port 3033 by default.  


Create your first character, following the character creation prompts.  
Once you are finished the character creation, type "relevel ChangeMe"  

You will  be immediately advanced to maximum level and have your security and trust set accordingly.   
   
I would recommend using the "cmdedit" command to disable the relevel command, or editing do_relevl  
 in act_wiz.c to change the password to something stronger than 'ChangeMe' because until you either  
 disable to command or disable it, anyone who logs in will be able to use the relevel command to   
 advance themselves to maximum level if they've read this README file. I would also consider setting  
 newlock on once your staff have all created characters until you are ready to open to the public.    
   
You are now ready to begin creating your world.  
  
Included by default is a crude world map of Faerun from the D&D Forgotten Realms campaign setting.  
  
  
  
  
  
## Notable changes from 'stock Rivers of MUD'  
### Level system  
There are 20 player levels. This is due to using the DnD3.5e EXP table for leveling.    
  
### Multiclassing   
Upon leveling, you must use the 'levelup' command. This will display a menu explaining  
that you must select whether to put your new level into your existing class, or choose a  
second (or third) class using the 'multiclass' command. e.g. 'multiclass 1' will add the  
new level to your primary class, while 'multiclass' followed by another number in the   
corresponding menu will put the level into a new class, to a total of three classes.  
Multiclassing gives you access to skills and spells from additional classes. But  
be forewarned; you need to plan your classes or you could end up spending too many  
levels on other classes and miss out on higher level skills/spells of your main class.  
You can only gain a **total** of 20 levels as a player character. E.g. if your main  
class is Mage, and you spend 5 levels on Fighter and 5 levels on Rogue, that will  
only leave you 10 levels for Mage, which limits you to 6th caster level spells. And  
there at 10 total caster levels!   
  
### Spell Memorization  
You can only cast spells that you have memorized from your spellbook. Each character level  
you gain additional spell slots for memorization, You gain a *caster level* approximately  
every two character levels. So a level 1 Mage can cast level 1 spells, a level 2 Mage can  
cast level 2 spells, a level 4 Mage can cast level 3 spells, a level 6 Mage can cast level  
4 spells, and so on, until level 20 when level 10 spells become available. The 'odd levels'  
add spell slots for the caster levels you've already gained, so while you do not gain a new  
*caster level* at level 5, you do gain additional spell slots for memorizing level 1-3 spells.  
You are able to use the 'forget' command to forget a memorized spell and free up a spell slot.  
  
### Advanced Weather and Survival Mechanics  
MARS sports a pretty robust weather system, complete with seasons, sunrise and sunset changing  
month-to-month, temperatures that change by the hour, rain, snow, sleet, heat exhaustion and  
a survival system to keep yourself alive in the harshest enviroments. Hunt animals for meat  
and furs, forage for berries, mushrooms, herbs, and root, gather wood to build a shelter or  
start a campfire, go fishing, cook your meat and fish with foraged ingredients and learn new  
recipes to share with your friends.  
  
### Potential for a Massive World!  
The overland system allows you to create huge world maps for players to explore. MARS comes  
with a world map of almost 700,000 rooms to start with! Endless potential for random encounters,  
puzzles, quests, and more!  
  
### Vehicles, Mounts and more!  
Vehicles from horse drawn carraiges, chariots, and even massive ships on the open ocean, as well  
as mounts will help players travel the world you build. The limit is your imagination.