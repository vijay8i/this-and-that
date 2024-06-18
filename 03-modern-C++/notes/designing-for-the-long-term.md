---
Title: Notes on good C++ design from the experts
Article-Type: Notes
Source-Attribution: 
Source-Urls:
  - https://www.youtube.com/watch?v=motLOioLJfg
  - https://www.youtube.com/watch?v=Wx9nzYTUd-c
---

# Designing Classes (Parts 1&2) - Klause Iglberger (CppCon 2021)
The Challenge of Class Design
- What is the root source of all problems in software development?
  - Change!
  - The truth in our industry: Software must be adaptable to frequent
    changes. It is not that simple, however.
  - 
- What is the core problem of adaptable software and software 
  development in general?
  - Dependencies!
  - "Dependency is the key problem in software development at all scales" (Kent Beck)
  - Guideline: Design classes for easy change and extension

Design for Readability
- Guideline: Spend time to find good names for all entities
- C++ `vector` has many bad examples of how not to name things
- Watch -> Naming is Hard: Let's Do Better (Kate Gregory, CppCon 2019)
- "Naming requires Empathy." (Kate Gregory)


Design for Change and Extension
- Use concepts to define traits, contracts
- Getting your class hierarchy right is a critical step at design time
- Using inheritance naively to solve problems easily leads to...
  - ... many derived classes
  - ... ridiculous class names
  - ... deep inheritance hierarchies
  - ... duplication between similar implementations
  - ... (almost) impossible extensions
  - ... impeded maintenance
- Guideline: Resist the urge to put everything into one class. Seperate concerns!
- If you use OO programming, use it properly.

The Solution: Design Principles and Patterns
"Inheritance is Rarely the Answer. Delegate to Services: Has-A Trumps Is-A" (Andrew Hunt, David Thomas)

- Single-Responsibility Principle (SRP)
"Everything should do just one thing." is a super vague idea and not practicable!
A better definition: The SRP advices to seperate concerns to **isolate and simplify change** (Klause Iglberger)
In practice, SRP is about:
  - seperation of concerns
  - high cohesion / low coupling
  - finding orthogonality
  - NOTE: A class that has two member resources is necessarily breaking
    the SRP because a C++ constructor can throw at any time during the
    initialization of the resources! 

- Open-Closed Principle (OCP)
OCP advices to prefer design that simplifies the extension by types or operations.
Open for extension, closed for modification is a good way to think about OCP.

- Don't Repeat Yourself (DRY)
The DRY principle advices to reduce duplication in order to simplify change.

- Learn from GoF Design Patterns (Not all of them may be applicable for C++)
GoF's 23 design patterns fall under one of creational, structural or
behavioural categories. A design pattern...
  - ... has a name
  - ... carries and intent
  - ... aims at reducing dependencies
  - ... provides some sort of abstraction
  - ... has proven to work over the years

Virtual functions almost always are a bad idea because they unwittingly
force an inheritance chain too early in the design phase when things are
still being discovered, and continue to propogage because no one dares
to make changes to the interface because of the cascading changeset that
such a change unleashes.


# Designing for the long term: invariants, knobs, extensions
ref: https://abseil.io/resources/swe-book/html/toc.html

Configuration should be:
- Orthogonal
- Focussed on outcomes/intents
- Minimal
- Easy to reason about

Experimentation, Release:
- Functionality gated by feature flags/configuration
- Management of that flag/configuration is controlled by:
  - Release engineers
  - Experimental frameworks (A/B tests)
  - Rollout systems
- Cleanu up. Promptly!!

Extension:
- Callbacks: be very precise about how you will invoke a callback.
  - Which threads(s)?
  - Locks held?
  - Order of invocation?
  - Frequency of invocation?
- Polymorphism:
  - Avoid?
  - PIMPL?
  - Proceed very carefully:
    - An abstract interface is both requirements and affordances, these
      are hard to change.
    - ABI lurks here in more ways
- Templates, Extension Point, etc:
  - Proceed with care
  - Document intent
  - Use concepts and lean on semantics:
    - example: std::accumulate was originally copied values; but it
      was able to change (to use/rely on move where appropriate) because
      the semantic did not imply a math operation to the exclusion of
      other meanings of accumulation.

Summary:
  - Configuration based on outcomes and intent
  - Customization fights optimization/maintenance
  - Extensible interfaces are hard to get right; and very hard to change
    after the fact
  - The popcorn button is a trap!