﻿#pragma once

#include "CoreMinimal.h"
#include "yaml.h"
#include "UnrealTypes.h"
#include "Enums.h"
#include "Emitter.h"

#include "Node.generated.h"

class FYamlIterator;


/** A wrapper for the Yaml Node class. Base YAML class. Stores a YAML-Structure in a Tree-like hierarchy.
 * Can therefore either hold a single value or be a Container for other Nodes.
 * Conversion from one Type to another will be done automatically as needed
 */
USTRUCT(BlueprintType)
struct UNREALYAML_API FYamlNode {
    GENERATED_BODY()

private:
    friend void operator<<(std::ostream& Out, const FYamlNode& Node);
    friend void operator<<(FYamlEmitter& Out, const FYamlNode& Node);

    YAML::Node Node;

public:
    // Constructors --------------------------------------------------------------------
    /** Generate an Empty YAML Node */
    FYamlNode() = default;

    /** Generate an Empty YAML Node of a specific Type */
    explicit FYamlNode(const EYamlNodeType Type) :
        Node(static_cast<YAML::NodeType>(Type)) {}

    /** Generate a YAML Node that contains the given Data, which is implicitly Converted */
    template<typename T>
    explicit FYamlNode(const T& Data) :
        Node(Data) {}

    /** Generate an YAML Node from an Iterator Value */
    explicit FYamlNode(const YAML::detail::iterator_value& Value) :
        Node(YAML::Node(Value)) {}

    /** Generate an YAML Node from a Native YAML Node*/
    explicit FYamlNode(const YAML::Node Value) :
        Node(Value) {}

    // Types ---------------------------------------------------------------------------
    /** Returns the Type of the Contained Data */
    EYamlNodeType Type() const;

    /** If the Node has been Defined */
    bool IsDefined() const;

    /** Equivalent to Type() == Null (No Value) */
    bool IsNull() const;

    /** Equivalent to Type() == Scalar (Singular Value) */
    bool IsScalar() const;

    /** Equivalent to Type() == Sequence (Multiple Values without Keys) */
    bool IsSequence() const;

    /** Equivalent to Type() == Map (List of Key-Value Pairs) */
    bool IsMap() const;


    // Conversion to bool and output to a Stream ---------------------------------------
    explicit operator bool() const;
    bool operator !() const;

    // Style ---------------------------------------------------------------------------
    /** Returns the Style of the Node, mostly relevant for Sequences */
    EYamlEmitterStyle Style() const;

    /** Sets the Style of the Node, mostly relevant for Sequences */
    void SetStyle(const EYamlEmitterStyle Style);


    // Assignment ----------------------------------------------------------------------
    /** Test if 2 Nodes are Equal */
    bool Is(const FYamlNode& Other) const;
    bool operator ==(const FYamlNode Other) const;

    /** Assign a Value to this Node. Will automatically converted */
    template<typename T>
    FYamlNode& operator=(const T& Value) {
        try {
            Node = Value;
        } catch (YAML::InvalidNode e) {
            UE_LOG(LogTemp, Warning, TEXT("Node was Invalid, won't assign any Value!"))
        }
        return *this;
    }

    /** Assign a Value to this Node. Will automatically converted */
    FYamlNode& operator=(const FYamlNode& Other) {
        Node = Other.Node;
        return *this;
    }

    /** Overwrite the Contents of this Node with the Content of another Node, or delete them if no Argument is given
     *
     * @returns If the Operation was successful
     */
    bool Reset(const FYamlNode& Other = FYamlNode());


    // Access --------------------------------------------------------------------------
    /** Try to Convert the Contents of the Node to the Given Type or a nullptr when conversion is not possible
     *
     * @return A Pointer to the Converted Value. If the Conversion was unsuccessful, return a nullptr
     */
    template<typename T>
    TOptional<T> AsOptional() const {
        try {
            return Node.as<T>();
        } catch (YAML::Exception) {
            return {};
        }
    }

    /** Try to Convert the Contents of the Node to the Given Type or return the Default Value
    * when conversion is not possible.  Serves as a more Secure version of AsPointer()
    *
    * The DefaultValue will default to the default Constructor of the Type. If this Constructor is not available,
    * you must supply a DefaultValue manually
    * 
    * @return A Reference to the Converted Value. If the Conversion was unsuccessful, return the DefaultValue
    */
    template<typename T>
    T As(T DefaultValue = T()) const {
        try {
            return Node.as<T>();
        } catch (YAML::Exception) {
            return DefaultValue;
        }
    }

    /** Check if the given node can be converted to the given Type */
    template<typename T>
    bool CanConvertTo() const {
        try {
            Node.as<T>();
            return true;
        } catch (YAML::Exception) {
            return false;
        }
    }

    /** Try to Content of the Node if it is a Scalar */
    FString Scalar() const;

    /** Returns the whole Content of the Node as a single FString */
    FString GetContent() const;

    // Size and Iteration --------------------------------------------------------------
    /** Returns the Size of the Node if it is a Sequence or Map, 0 otherwise */
    int32 Size() const;

    /** Returns the start for an iterator. Use in combination with end() */
    FYamlIterator begin();

    /** Returns the end for a iterator. Use in combination with begin() */
    FYamlIterator end();

    // Sequence ------------------------------------------------------------------------
    /** Converts the Node to a Sequence and adds the Element to this list */
    template<typename T>
    void Push(const T& Element) {
        try {
            Node.push_back(Element);
        } catch (YAML::InvalidNode) {
            UE_LOG(LogTemp, Warning, TEXT("Node was Invalid, can't Push any Value onto it!"))
        }
    }

    /** Converts the Node to a Sequence and adds the Node to this list */
    void Push(const FYamlNode& Element) {
        try {
            Node.push_back(Element.Node);
        } catch (YAML::InvalidNode) {
            UE_LOG(LogTemp, Warning, TEXT("Node was Invalid, can't Push any Value onto it!"))
        }
    }

    // Map -----------------------------------------------------------------------------
    /** Forces a Conversion to a Map and adds the given Key-Value pair to the Map */
    template<typename K, typename V>
    void ForceInsert(const K& Key, const V& Value) {
        Node.force_insert(Key, Value);
    }

    // Indexing ------------------------------------------------------------------------
    /** Returns the Value at the given Key or Index */
    template<typename T>
    const FYamlNode operator[](const T& Key) const {
        return FYamlNode(Node[Key]);
    }

    /** Returns the Value at the given Key or Index */
    template<typename T>
    FYamlNode operator[](const T& Key) {
        return FYamlNode(Node[Key]);
    }

    /** Removes the Value at the given Key or Index */
    template<typename T>
    bool Remove(const T& Key) {
        return Node.remove(Key);
    }

    /** Returns the Value at the given Key or Index */
    const FYamlNode operator[](const FYamlNode& Key) const {
        return FYamlNode(Node[Key.Node]);
    }

    /** Returns the Value at the given Key or Index */
    FYamlNode operator[](const FYamlNode& Key) {
        return FYamlNode(Node[Key.Node]);
    }

    /** Removes the Value at the given Key or Index */
    bool Remove(const FYamlNode& Key) {
        return Node.remove(Key.Node);
    }
};

// Global Variables --------------------------------------------------------------------

/** Write the Contents of the Node to an OutputStream */
inline void operator<<(std::ostream& Out, const FYamlNode& Node) {
    Out << Node.Node;
}

/** Write the Contents of the Node into an Emitter */
inline void operator<<(FYamlEmitter& Out, const FYamlNode& Node) {
    Out << Node.Node;
}

// Iterator ----------------------------------------------------------------------------

/** The Iterator Base class. */
class FYamlIterator {
    friend FYamlNode;

    YAML::iterator Iterator;
    int32 Index;

    explicit FYamlIterator(const YAML::iterator Iter) :
        Iterator(Iter),
        Index(0) {}

    // Proxystruct returned by the -> operator
    struct FProxy {
        FYamlNode Ref;

        explicit FProxy(FYamlNode& Node) :
            Ref(Node) {}

        FYamlNode* operator->() {
            return &Ref;
        }

        operator FYamlNode*() {
            return &Ref;
        }
    };

public:
    /** Returns the <b>Key</b> Element of the Key-Value-Pair if the Iterated Node is a <b>Map</b>
     * or a Node containing the <b>Index</b> of the Value if the Iterated Node is a <b>List</b>!
     *
     * The corresponding Value can be retrieved via Value() */
    FYamlNode Key() {
        if (Iterator->first.IsDefined()) {
            return FYamlNode(Iterator->first);
        }

        return FYamlNode(Index);
    }


    /** Returns the <b>Value</b> Element of the Key-Value-Pair if the Iterated Node is a <b>Map</b>
    * or a Node containing the <b>Value</b> if the Iterated Node is a <b>List</b>!
    *
    * The corresponding Key (for a Map) or Index (for a List) can be retrieved via Key() */
    FYamlNode Value() {
        if (Iterator->second.IsDefined()) {
            return FYamlNode(Iterator->second);
        }

        return **this;
    }


    /** Dereferencing the Iterator yields the Value */
    FYamlNode& operator*() const {
        if (Iterator->second.IsDefined()) {
            return *new FYamlNode(Iterator->second);
        }

        return *new FYamlNode(*Iterator);
    }


    /** The Arrow Operator yields a Pointer to the Value */
    FProxy& operator->() const {
        return *new FProxy(**this);
    }


    FYamlIterator& operator++() {
        ++Iterator;
        Index++;
        return *this;
    }


    FYamlIterator operator++(int) {
        FYamlIterator Pre(*this);
        ++(*this);
        Index++;
        return Pre;
    }

    bool operator ==(const FYamlIterator Other) const {
        return Iterator == Other.Iterator;
    }

    bool operator !=(const FYamlIterator Other) const {
        return Iterator != Other.Iterator;
    }
};
