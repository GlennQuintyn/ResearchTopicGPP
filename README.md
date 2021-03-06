# ResearchTopicGPP


## Mini Formation Battle Simulator

## Intro

### The Idea
The idea for this research project is to make a small and simple battle simulator. The goal is to let 2 groups of equal manpower and equal ability to take and deal damage, battle it out in a small area.
The only variables you can play with are the formation each team uses and their specific start point (within a certain start box). 

### The Implementation
For this research project I used the Elite Engine **G**ame **P**lay **P**rogramming framework. Original authors: Matthieu Delaere, Thomas Goussaert with added code from Yosha Vandaele and Andries Geens. The code it self is based on the flocking project we had to make but I modified it, so I could add in the 2 teams and fighting. I used the steering Behaviours that we made for the flocking assigment and in the beginning of GPP to move the agents around in a simple way an army would move.

## Steering Behaviours
### Priority Steering Behaviour
This steering behaviour exists out of 2 steering behaviours where one has priority over the other (hence the name). The first one (the one that has priority over the other behaviour) is an attack beviour. This behaviour deals with the enemies when they are in vision range or in melee range to attack them. The other part is a blended steering behaviour. 
```cpp
//priority steering
m_pBluePrioritySteering = new PrioritySteering({ {m_pAttackBehavior},{m_pBlueBlendedSteering} });
```

### Blended Steering Behaviour
This steering behaviour is as its name implies a blend of different steering behaviours. The blend exists out of 4 different steering behaviours, each with a weight value that tells how much of this behaviour needs to be used:
 * Seperation Behavior: Makes sure the agents don't clump together and take some distance from each other. 
 * Cohesion Behavior: Makes sure the agents don't go too far away from each other. 
 * Velocity Match Behavior: The agents try to match the velocity of the other agents around them to move more as a unit instead of individual agents. 
 * Seek Behavior: The agents need a target to go to and that is their position in their formation. 
```cpp
//blended steering
m_pBlueBlendedSteering = new BlendedSteering({ { m_pSeperationBehavior, 0.56f }, { m_pCohesionBehavior, 0.55f }
		, { m_pVelMatchBehavior, 0.35f } , { m_pBlueSeekBehavior, 0.9f } });//implicit vetor of weighted behavior
```

## Setting Up The Battle
In the info panel on the right, you can select which formation each team is going to use.

<img src="https://github.com/GlennQuintyn/ResearchTopicGPP/blob/master/FormationSelection.PNG" alt="Formation Selectin img" width="300" height="200"/>

When that is selected, the previous agents will be removed and new agents will be spawned in randomly from the somewhere within the spawn zone.

![Formation forming gif](https://github.com/GlennQuintyn/ResearchTopicGPP/blob/master/Formation.gif)

You can click with left mouse button to change the middle point of the group formation to somwehere in their spawn zone. The same can be done for the red team but then you have to press middle mouse button.
When everything is setup you press the `FIGHT` button and the battle begins. At the end a small statistics window will be shown.

![Formation Selectin img](https://github.com/GlennQuintyn/ResearchTopicGPP/blob/master/BattleEndCard.PNG)

There is also a `RESET` button which will reset both teams and respawn them so you can test a different formation with previous settings rememberd.


## Formations
In my current project I only had time to implement 3 different formations each of 36 manpower big.

* 6 x 6 Phalanx

![Formation Selectin img](https://github.com/GlennQuintyn/ResearchTopicGPP/blob/master/6x6%20Phalanx.PNG)

* Flying Wedge

![Formation Selectin img](https://github.com/GlennQuintyn/ResearchTopicGPP/blob/master/Flying%20Wedge.PNG)

* Wedge Phalanx

![Formation Selectin img](https://github.com/GlennQuintyn/ResearchTopicGPP/blob/master/Wedge%20Phalanx.PNG)

## References

### Other Info
* [Meaning of Phalanx](https://www.wikiwand.com/simple/Phalanx_formation#:~:text=The%20phalanx%20formation%20is%20an,who%20often%20fought%20each%20other)

### formations
* [6 x 6 Phalanx](https://en.wikipedia.org/wiki/Phalanx)
* [Flying Wedge](https://en.wikipedia.org/wiki/Flying_wedge)
* [Wedge Phalanx](https://www.quora.com/Why-was-the-Macedonian-phalanx-so-effective-in-Alexanders-time-and-so-vulnerable-against-the-Romans)
