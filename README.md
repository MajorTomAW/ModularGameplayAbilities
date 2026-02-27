# Modular Gameplay Abilities

## <a id="head"></a> Overview

Unreal Engine Plugin for extending the GAS in a Lyra-like manner.
The reason this plugin exists, is that I find myself very often writing boilerplate to setup GAS for my projects.
Therefore, my aim was to provide a base GAS setup, that can be used for (almost) any type of project.

(I don't really like the name "Modular Gameplay Abilities" anymore lol, I was feeling a bit too fancy when I chose that name.)



> [!WARNING]
> This is still in development and some features may not work as expected.

---

## Index
> 1. [Modular Gameplay Ability](#modular-gameplay-ability)<br>
> &nbsp; 1.1 [Activation](#mga-activation)<br>
> &nbsp; 1.2 [Ability Input](#mga-ability-input)<br>
> &nbsp; 1.3 [(Explicit) Cooldowns](#mga-cooldowns)<br>
> &nbsp; 1.4 [AI-Controlled Ability usage](#mga-ai-controlled)
> 2. [Modular Ability System Component](#modular-asc)<br>
> &nbsp; 2.1 [Lazy-Loading the ASC](#asc-lazyloading)
> 3. [Modular Attribute Set](#modular-attribute-set)
> 3. [Ability Tasks](#ability-tasks)
> 100. [Installing the plugin](#installing)

<!-- &nbsp; 1.2 [Actor Tracking](#mga-actor-tracking)<br>
> &nbsp; 1.3 [Failure Management](#mga-failure-management)<br>-->

---


<a name="modular-gameplay-ability"></a>
## Modular Gameplay Ability
The _[UModularGameplayAbilty](Source/ModularGameplayAbilities/Public/Abilities/ModularGameplayAbility.h)_ is an extended version of the base ``UGameplayAbility`` providing more functionality and customization options in the context of activation, failure, cooldowns, etc.

---

<a name="mga-activation"></a>
### Activation
Gameplay Ability activation can happen in 3 different ways:

| Activation Policy | Description |
| -------- | ---------------- |
| 1. Passive  |  Used for abilities that always apply gameplay effects and/or tags when being given to an ASC. (e.g. "On Spawn") |
| 2. Triggered | Abilities that should be activated by a trigger (for example a GameplayEvent / Gameplay Message / etc.) |
| 3. Active | Abilities that should explicitly be activated by player actions. (E.g. pressing an input key) |

> [!NOTE]
> Triggered or Passive abilities won't receive Input events, unless ``ForceReceiveInput`` is turned on.

![image](https://github.com/user-attachments/assets/f7fafb5b-391c-48c4-a790-bc81c6752c6c)

Read more about abilit yinput [here](#mga-ability-input)

This will run through the default ability activation process but also checking for its Activation Group.
The principle of Activation Groups is pretty much ported from Lyra.

On a high level, each ability can have its own activation group the defines its relationship to other abilities.

| Activation Group           | Description                                                                                                                                                                                                 |
|----------------------------|-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| 1. Independent             | Usually the majority of your abilities will run independently from another. Meaning they don't care about the activation of other abilities.                                                                |
| 2. Exclusive (Replaceable) | Setting the Activation Group to "Exclusive (Replaceable) means there should only be one exclusive ability running at a time. **But** it can be canceled and replaced by other exclusive abilities.          |
| 3. Exclusive (Blocking)    | This does pretty much the same as the "Exclusive (Replaceable) Activation Group, however instead of getting canceled by others, this will actively **block** all other exclusive abilities from activating. |


Potential usage of this could be:
- **Independent**: Abilities that can be used at any time, e.g. ``Death``, ``Respawn``, ``Do A Thing``.
- **Exclusive (Replaceable)**: Abilities that can be replaced by others, e.g. ``Aim Down Sights``, ``Sprint``, ``Crouch``.
- **Exclusive (Blocking)**: Abilities that should block others, e.g. ``Show Inventory``, ``Show Map``. (Only one at a time should be shown)

![image](https://github.com/user-attachments/assets/5812b4f2-693c-498b-b085-638b29e515ce)


---
<a name="mga-ability-input"></a>
### Ability Input

For convenience, the ``UModularGameplayAbility`` comes with built-in "input released/pressed" methods,<br>
which will be triggered according to the ability's ``Activation Policy``.<br>
Both native and blueprint versions.
```cpp
virtual void OnAbilityInputPressed(float TimeWaited);
virtual void OnAbilityInputReleased(float TimeHeld);
```

![image](https://github.com/user-attachments/assets/517d9fbb-bbd7-44b9-bfae-bc2e7dd759b5)

---

<!---

<a name="mga-actor-tracking"></a>
### 📣 〢 Actor Tracking
[...]

---

<a name="mga-failure-management"></a>
### 📣 〢 Failure Management
[...]

--->

<a name="mga-cooldowns"></a>
### (Explicit) Cooldowns

The ``ExplicitCooldownDuration`` property can be used to specifiy per-ability cooldowns.
This will be injected into the provied Cooldown Gameplay Effect class, together with the Explicit Cooldown tags and Asset Tags.

Therefore, allowing you to share the same Cooldown Gameplay Effect with multiple Abilities.

> [!WARNING]
> Since ``CheckCooldown()`` checks for cooldown tags (applied from the Cooldown Gameplay Effect), you must have unique Explicit Cooldown tags PER ability.
> Otherwise the cooldown might function as a global cooldown, blocking all abilities, or cause unexpected behavior.
>
> (The ``UModularGameplayAbility`` overrides the ``GetCooldownTags()``, appending the Explicit Cooldown tags.)

![image](https://github.com/user-attachments/assets/d9afa9a5-6aaa-4f92-8366-8968460b44de)

---

<a name="mga-ai-controlled"></a>
### AI-Controlled Ability usage
As Gameplay Abilities are also able to be activated by bot-controlled pawns, the ``UModularGameplayAbility`` provides further logic modifying the AI's behavior.<br>
E.g. during the Ability activation, if instigated by an AI, it will stop any Behavior Logic / AI Movement / RVO Avoidance / ... <br>
To enable those "AI Events", you need to check the ``TriggerAIEvents`` property.

![image](https://github.com/user-attachments/assets/2005f113-5dca-46bd-ba86-90e67a74c0f4)

By default, the following AI events are implemented:
| Property | Description |
| -------- | ---------------- |
| Stops AI Behavior Logic | Stops AI Behavior logic until the abilit is finished/aborted. |
| Stops AI Movement | Will pause the current AI move until the ability is finished/aborted. |
| Stops AI RVO Avoidance | Not implemented yet |
| Activation Noise Range | When greater 0, will report a Noise event to the ``UAISense_Hearing`` sense, using the provided Range as radius and the ``Activation Noise Loudness`` as Loudness. |
| Impact Noise Range | Not implement yet |
| Activation Noise Loudness | @see ``Activation Noise Range`` |

Further AI Events can be implemented by overriding ``TriggerAIEventsOnActivate/Deactivate`` in either C++ or BP.<br>
![image](https://github.com/user-attachments/assets/217db4c5-892f-447d-b602-dcac7759110c)

---

<a name="asc-lazyloading"></a>
## 2. Lazy-Loading the ASC
The _[AModularAbilityActor](Source/ModularGameplayAbilities/Public/ModularAbilityActor.h)_ provides logic for an actor that is meant to use the Ability System.
Thus setting up an example of how to lazy-load the Ability System and manage pending attribute modifiers for optimal performance.
[...]

![image](https://github.com/user-attachments/assets/2557c7a8-4154-44ea-816b-87c760f965f5)

[Credit (Vorixo)](https://vorixo.github.io/devtricks/lazy-loading-asc/)

---

<a name="ability-tasks"></a>
#

---

<p align="center">
If you like this project, leaving a star is much appreciated<br>
<a href="#head">
Back to the top
</a>
</p>
