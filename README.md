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

### 2.4. Utilizing `PackageAdapter` for Targeted Transfers

The `PackageAdapter` is a crucial component that allows a standalone `Container` to participate in `Package`-level operations by making it conform to the `PackageInterface`. This is especially useful for targeted resource transfers.

Consider this example:

```cpp
    // 1. Initialize a provider package with capacities
    Package providerPackage { std::unordered_map<EResource, Package::Units> {
            { EResource::Type1, 255.f}, // Container for EResource::Type1 with 255 units capacity
            { EResource::Type2, 255.f}, // Container for EResource::Type2 with 255 units capacity
        } };
```

// 2. Ensure provider's containers are full (redundant if constructor fills them, but good for clarity)

```cpp
    providerPackage.GetContainer(EResource::Type1).SetAmount(255.f);
    providerPackage.GetContainer(EResource::Type2).SetAmount(255.f);
```

// 3. Create a standalone consumer container with its own capacity

```cpp
    Package::Container consumerContainer { 255.f }; // This container is currently empty by default (or needs explicit SetAmount(0.f))
```

// 4. Perform a targeted transfer using PackageAdapter
//    This line transfers only EResource::Type1 from providerPackage into consumerContainer.
//    The PackageAdapter acts as a temporary bridge, mapping consumerContainer to the Type1 resource.

```cpp
    providerPackage >> PackageAdapter(EResource::Type1, consumerContainer);
```

**Breakdown of the `PackageAdapter` transfer:**

* `providerPackage >> ...`: Initiates a transfer *from* `providerPackage`.
* `PackageAdapter(EResource::Type1, consumerContainer)`: A temporary `PackageAdapter` is created. It wraps `consumerContainer` and declares that this container should be treated as the target for `EResource::Type1`.
* **Result**:
    * `providerPackage`'s internal container for `EResource::Type1` will become empty (0 units available).
    * `providerPackage`'s internal container for `EResource::Type2` remains unaffected (still full with 255 units).
    * `consumerContainer` will become full (255 units available), having received the `Type1` resource.

This demonstrates how `PackageAdapter` enables a single, general-purpose `Container` to interact with a multi-resource `Package` for a specific resource type, acting as a flexible conduit for resource flow.

---

## 3. Potential Use Cases

The `Vessel` API's structure makes it suitable for various resource management scenarios, including:

* **Game Development**: Managing player inventories, in-game resources (health, mana, ammunition), or crafting materials.
* **Simulation**: Tracking resource consumption and production in complex simulated environments.
* **Logistics & Manufacturing**: Modeling stock levels, material flow, or contents of storage units.

---
