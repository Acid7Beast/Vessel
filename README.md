# Vessel API: Resource Management System

This document describes the `Vessel` API, a C++ system designed for managing and tracking various **resources** and **units** within **containers** and **packages** as streams. It utilizes a flexible, templated approach, making it adaptable for different resource types and units. The API also supports robust testing, demonstrated by the provided unit test examples.

---

## 1. Core Concepts

The `Vessel` API revolves around the following fundamental ideas:

* **Tags (`PackTestTag`)**: These are empty struct used as **template parameters** to specialize the behavior of `Vessel` components. This allows for distinct configurations for different "types" of packages or resources. For `PackTestTag`, the `TagSelector` specialization dictates that `Units` are `float` and `ResourceId` is `TestResource`.

* **Resources (`TestResource`)**: An `enum class` (e.g., `TestResource::Test1`, `TestResource::Test2`) that **identifies specific types of resources** being managed.

* **Units (`float`)**: The **quantitative measure** of the resources. In the `PackTestTag` specialization, units are represented as `float` or `int` (e.g., `kEmptyAmountKg`, `kCapacityAmountKg`, `kEmptyPoints`, `kCapacityPoints`).

* **Containers (`Vessel::Container<Tag>`)**: These represent **individual storage units** for a specific resource type. They typically track `RequestUnits` (how much has been taken or requested) and `AvailableUnits` (how much is remaining or can be supplied). A container is essentially a buffer for a single resource type.

* **Packages (`Vessel::Package<Tag>`)**: These act as **collections of containers**. A `Package` manages multiple `Container` instances, each corresponding to a `ResourceId`. They provide mechanisms for loading, saving, and exchanging resource states among themselves or with other containers.

* **PackageInterface (`Vessel::PackageInterface<Tag>`)**: This is an interface (likely an abstract base class or concept) that both `Package` and `PackageAdapter` adhere to. It enables **polymorphic interaction**, meaning you can write code that operates on any object implementing this interface without needing to know its concrete type.

* **PackageAdapter (`Vessel::PackageAdapter<Tag>`)**: This powerful class allows an **individual `Container` to be treated as a `PackageInterface`**. This is particularly useful for applying operations designed for packages (like resource transfers) to single containers by specifying which resource type that container represents for the operation.

---

## 2. Working with the API

The provided code examples and tests demonstrate key API interactions.

### 2.1. Initialization and State Management

* **Creating a Package**:
    You initialize a `Package` by providing an `std::unordered_map` that defines the **initial capacity for each `ResourceId`** its internal containers will hold.

    ```cpp
    // Define initial capacities for resources
    static const std::unordered_map<ResourceId, Package::Units> kContainerProperties
    {
        { ResourceId::Test1, kCapacityAmountKg}, // Test1 has kCapacityAmountKg
        { ResourceId::Test2, kCapacityAmountKg}, // Test2 has kCapacityAmountKg
    };

    // Create a package. By default, its internal containers are full upon creation.
    Package consumerPackage{ kContainerProperties };
    ```

* **Loading State**:
    You can set the state of multiple containers within a package using `LoadState`. This method takes a `ContainerStateTable` (a map of `ResourceId` to the desired *requested* amount of units). The `LoadState` effectively updates the `RequestUnits` for each specified container.

    ```cpp
    const Package::ContainerStateTable emptyStateTable
    {
        {ResourceId::Test1, { kEmptyAmountKg }}, // Set Test1 to empty (all requested)
        {ResourceId::Test2, { kEmptyAmountKg }}, // Set Test2 to empty (all requested)
    };

    consumerPackage.LoadState(emptyStateTable);
    // After this, consumerPackage's containers for Test1 and Test2 will be in an "empty" state.
    ```

* **Saving State**:
    To retrieve the current "requested" state of a package, use `SaveState`. This populates a `ContainerStateTable` with the current `RequestUnits` for each resource.

    ```cpp
    Package::ContainerStateTable currentPackageState;
    consumerPackage.SaveState(currentPackageState);
    // currentPackageState now holds the requested amounts for all resources in consumerPackage.
    ```

### 2.2. Accessing and Modifying Containers Directly

You can obtain a mutable reference to a specific container within a package using `GetContainer` and then directly modify its `Amount`.

```cpp
    // Get the container for Test1 and set its current amount to 0 (empty)
    consumerPackage.GetContainer(ResourceId::Test1).SetAmount(kEmptyAmountKg);
    // This directly manipulates the available units within the container.
```

### 2.3. Exchanging Resources Between Packages

The API provides convenient overloaded operators (`>>` and `<<`) for transferring resources between `PackageInterface` objects.

* **Transfer from Provider to Consumer (`>>`)**:
    The `providerPackage >> consumerPackage;` operation transfers all available resources from `providerPackage` to `consumerPackage`. After this, `providerPackage`'s containers will be empty, and `consumerPackage`'s containers will be full.

    ```cpp
    providerPackage >> consumerPackage;
    // consumerPackage's containers are now full, providerPackage's are empty.
    ```

* **Transfer from Consumer to Provider (`<<`)**:
    The `providerPackage << consumerPackage;` operation transfers all available resources from `consumerPackage` to `providerPackage`. After this, `consumerPackage`'s containers will be empty, and `providerPackage`'s containers will be full.

    ```cpp
    providerPackage << consumerPackage;
    // consumerPackage's containers are now empty, providerPackage's are full.
    ```

---

### 2.4. Utilizing `PackageAdapter` for Targeted Transfers

The `PackageAdapter` is a crucial component that allows a standalone `Container` to participate in `Package`-level operations by making it conform to the `PackageInterface`. This is especially useful for targeted resource transfers.

### 2.4. Utilizing `PackageAdapter` for Targeted Transfers

The `PackageAdapter` is a crucial component that allows a standalone `Container` to participate in `Package`-level operations by making it conform to the `PackageInterface`. This is especially useful for targeted resource transfers.

Consider this example:

1. **Initialize a provider package with capacities**

    ```cpp
    Package providerPackage { std::unordered_map<EResource, Package::Units> {
        { EResource::Type1, 255.f }, // Container for EResource::Type1 with 255 units capacity
        { EResource::Type2, 255.f }, // Container for EResource::Type2 with 255 units capacity
    } };
    ```

2. **Ensure provider's containers are full**  
   *(This is redundant if the constructor fills them, but good for clarity)*

    ```cpp
    providerPackage.GetContainer(EResource::Type1).SetAmount(255.f);
    providerPackage.GetContainer(EResource::Type2).SetAmount(255.f);
    ```

3. **Create a standalone consumer container with its own capacity**

    ```cpp
    Package::Container consumerContainer { 255.f }; // This container is currently empty by default (or needs explicit SetAmount(0.f))
    ```

4. **Perform a targeted transfer using `PackageAdapter`**  
   This line transfers only `EResource::Type1` from `providerPackage` into `consumerContainer`.  
   The `PackageAdapter` acts as a temporary bridge, mapping `consumerContainer` to the `Type1` resource.

    ```cpp
    providerPackage >> PackageAdapter(EResource::Type1, consumerContainer);
    ```

---

**Breakdown of the `PackageAdapter` transfer:**

- `providerPackage >> ...`: Initiates a transfer *from* `providerPackage`.
- `PackageAdapter(EResource::Type1, consumerContainer)`:  
  A temporary `PackageAdapter` is created. It wraps `consumerContainer` and declares that this container should be treated as the target for `EResource::Type1`.
- **Result**:
  - `providerPackage`'s internal container for `EResource::Type1` will become empty (0 units available).
  - `providerPackage`'s internal container for `EResource::Type2` remains unaffected (still full with 255 units).
  - `consumerContainer` will become full (255 units available), having received the `Type1` resource.

This demonstrates how `PackageAdapter` enables a single, general-purpose `Container` to interact with a multi-resource `Package` for a specific resource type, acting as a flexible conduit for resource flow.

---

## 3. Potential Use Cases

The `Vessel` API's structure makes it suitable for various resource management scenarios, including:

* **Game Development**: Managing player inventories, in-game resources (health, mana, ammunition), or crafting materials.
* **Simulation**: Tracking resource consumption and production in complex simulated environments.
* **Logistics & Manufacturing**: Modeling stock levels, material flow, or contents of storage units.

---

## 4. Future Plans and Enhancements

Your proposed plans aim to significantly upgrade the `Vessel` API by leveraging advanced C++ features for better performance, compile-time safety, and developer experience.

### 4.1. Optimize with Static Polymorphism using CRTP

Using the **Curiously Recurring Template Pattern (CRTP)** will enable **static polymorphism**. This means that function calls can be resolved at **compile-time** rather than runtime, eliminating the overhead of virtual function calls associated with dynamic polymorphism.

* **How it helps**:
    * **Performance**: Reduces runtime overhead, leading to faster execution, especially for frequent operations on `PackageInterface`.
    * **Optimization**: Allows compilers to perform more aggressive optimizations like inlining, as the exact type is known during compilation.

### 4.2. Apply SBO Optimization, When Possible

**Small Buffer Optimization (SBO)** is a technique typically used for containers (like `std::string` or `std::vector`) to avoid dynamic memory allocation for small objects. If the size of the object (or its contained data) fits within a pre-allocated small buffer on the stack, it prevents a heap allocation, which can be a significant performance win.

* **How it helps**:
    * **Performance**: Reduces memory allocation overhead, particularly for `Container` or `Package` instances that manage a small number of resources or when their internal data structures are small. This can decrease cache misses and improve overall speed.
    * **Memory Efficiency**: Avoids heap fragmentation for frequently created small objects.

### 4.3. Add Concepts as Contracts

C++ **Concepts** allow you to define compile-time constraints on template parameters. Applying them as contracts will ensure that any types used with your `Vessel` templates (e.g., for `TagSelector`, `Units`, `ResourceId`) adhere to specific requirements.

* **How it helps**:
    * **Type Safety**: Guarantees that only valid types are used with your templates, catching errors at **compile-time** rather than runtime.
    * **Improved Error Messages**: Provides much clearer and more concise error messages to users when they violate type requirements, significantly improving the developer experience.
    * **User Guidance**: Clearly communicates the expected interfaces and properties of template arguments, serving as built-in documentation.

### 4.4. Make Possible of Compile-Time Usage

Enabling **compile-time usage** means allowing certain operations or resource definitions to be evaluated and fixed during compilation. This often involves using `constexpr` functions and variables, and potentially `std::integral_constant` or similar techniques.

* **How it helps**:
    * **Performance**: Moves computations from runtime to **compile-time**, resulting in zero-cost abstractions and faster program startup.
    * **Resource Definition**: Allows resource properties, capacities, or initial states to be immutable and known at compile time, which can simplify some logic and provide stronger guarantees.
    * **Metaprogramming**: Opens up possibilities for advanced **template metaprogramming** techniques, enabling highly specialized and optimized code generation.

---
