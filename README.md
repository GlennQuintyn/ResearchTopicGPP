# ResearchTopicGPP


### Mini Formation Battle Simulator
 

### The Idea
The idea for this research project is to make a small and simple battle simulator. The goal is to let 2 groups of equal manpower and equal ability to take and deal damage fight it out.
The only variables you can play with are the formation each team uses and their specific start point (within a certain start box). 

### The Implementation
For this research project I used the Elite Engine **G**ame **P**lay **P**rogramming framework. Original authors: Matthieu Delaere, Thomas Goussaert with added code from Yosha Vandaele and Andries Geens. The code it self is based on the flocking project we had to make but I modified it a lot to add in the 2 teams and fighting. I used the steering Behaviours that we made for the flocking assigment and in the beginning of GPP to move the agents around in a simple way an army would move.

## Steering Behaviours
### Priority Steering Behaviour
This steering behaviour exists out of 2 steering behaviours where one has priority over the other (hence the name). The first one (the one)
to deal with enemies. The other part is a blended steering of 
```cpp
//priority steering
m_pBluePrioritySteering = new PrioritySteering({ {m_pAttackBehavior},{m_pBlueBlendedSteering} });
```

### Blended Steering Behaviour

```cpp
//blended steering
	m_pBlueBlendedSteering = new BlendedSteering({ { m_pSperationBehavior, 0.56f }, { m_pCohesionBehavior, 0.55f }//0.55f
		, { m_pVelMathcBehavior, 0.35f } , { m_pBlueSeekBehavior, 0.9f } });//implicit vetor of weighted behavior

	m_pRedBlendedSteering = new BlendedSteering({ { m_pSperationBehavior, 0.56f }, { m_pCohesionBehavior, 0.55f }//0.55f
		, { m_pVelMathcBehavior, 0.35f } , { m_pRedSeekBehavior, .9f } });//implicit vetor of weighted behavior


	m_pRedPrioritySteering = new PrioritySteering({ {m_pAttackBehavior},{m_pRedBlendedSteering} });
```



When everything is setup you press the `FIGHT` button and the battle begins. At the end a small statistics window I shown.

fight it out on a battle field with the only difference being their formation and start position at the beginning of the battle.



> test




## References

### idk yet
* [smt](https://www.wikiwand.com/simple/Phalanx_formation#:~:text=The%20phalanx%20formation%20is%20an,who%20often%20fought%20each%20other)

### formations
* [6 x 6 Phalanx](https://en.wikipedia.org/wiki/Phalanx)
* [Flying Wedge](https://en.wikipedia.org/wiki/Flying_wedge)
* [Wedge Phalanx](https://www.quora.com/Why-was-the-Macedonian-phalanx-so-effective-in-Alexanders-time-and-so-vulnerable-against-the-Romans)
