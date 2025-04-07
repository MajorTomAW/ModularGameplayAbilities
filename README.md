# Modular Gameplay Abilities

Unreal Engine Plugin for extending the GAS in a Lyra-like manner.

# 📖 Overview
> 1. [Modular Gameplay Ability](#modular-gameplay-ability)  
> &nbsp; 1.1 [Activation](#mga-activation)  
> &nbsp; 1.2 [Failure Management](#mga-failure-management)  
> &nbsp; 1.3 [(Explicit) Cooldowns](#mga-cooldowns)  
> &nbsp; 1.4 [AI-Controlled Ability usage](#mga-ai-controlled)  
> 2. [Lazy-Loading the ASC](#asc-lazyloading)  

<a name="modular-gameplay-ability"></a>  
## 1. Modular Gameplay Ability
The _[UModularGameplayAbilty](Source/ModularGameplayAbilities/Public/Abilities/ModularGameplayAbility.h)_ is an extended version of the base ``UGameplayAbility`` providing more functionality and customization options in the context of activation, failure, cooldowns, etc.  

<a name="mga-activation"></a>  
- ### 1.1 Activation
Gameplay Ability activation can happen in 3 different ways:  
| Activation Policy | Description |
| -------- | ---------------- |
| 1. Passive  e.g. "On Spawn"  |  Used for abilities that always apply gameplay effects and/or tags when being given to an ASC |
| 2. Triggered | Abilities that should be activated by a trigger (for example a GameplayEvent / Gameplay Message / etc.) |
| 3. Active | Abilities that should explicitly be activated by player actions. (E.g. pressing an input key) |
  
This will run through the default ability activation process but also checking for its Activation Group.
Activation Groups are [...]
| Activation Group | Description |
| -------- | ---------------- |
[...]

![image](https://github.com/user-attachments/assets/9dc85a1f-5803-4ff2-848c-0a28b6d88ae6)


<a name="mga-failure-management"></a>    
- ### 1.2 Failure Management
[...]

<a name="mga-cooldowns"></a>  
- ### 1.3 (Explicit) Cooldowns
[...]

![image](https://github.com/user-attachments/assets/f046a9c6-a541-44b7-97d3-d07281b9c848)

<a name="mga-ai-controlled"></a>  
- ### 1.4 AI-Controlled Ability usage
As Gameplay Abilities are also able to be activated by bot-controlled pawns, the ModularGameplayAbility provides further logic modifying the AI's behavior.  
E.g. during the Ability activation, if instigated by an AI, it will stop any Behavior Logic / AI Movement / RVO Avoidance / ...
[...]

![image](https://github.com/user-attachments/assets/95e978ba-3f17-44fb-9698-0b92750332dd)

<a name="asc-lazyloading"></a>  
## 2. Lazy-Loading the ASC
The _[AModularAbilityActor](Source/ModularGameplayAbilities/Public/ModularAbilityActor.h)_ provides logic for an actor that is meant to use the Ability System.
Thus setting up an example of how to lazy-load the Ability System and manage pending attribute modifiers for optimal performance.
[...]

![image](https://github.com/user-attachments/assets/2557c7a8-4154-44ea-816b-87c760f965f5)
