#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#pragma warning(disable:4996)

#define TABLE_SIZE 127

/* Each parcel will have a link to another node in the tree and 3 variables inside */
typedef struct Parcel {
    char* destination;
    int weight;
    float valuation;
    struct Parcel* next;
} Parcel;

/* Tree node */
typedef struct BSTNode {
    Parcel* parcel;
    struct BSTNode* left;
    struct BSTNode* right;
} BSTNode;

/* 127 hash nodes will store 127 roots for 127 BSTs */
typedef struct HashNode {
    BSTNode* root;
} HashNode;

/* Function prototypes */
void traverseBST(BSTNode* node, int& totalWeight, float& totalValuation);
HashNode* initializeHashTable(void);
unsigned long computeHash(const char* str);
Parcel* createParcel(const char* destination, int weight, float valuation);
BSTNode* insertBST(BSTNode* root, Parcel* parcel);
void loadData(const char* filename, HashNode* hashTable);
void printParcels(BSTNode* root);
void searchByCountry(const char* country, HashNode* hashTable);
void searchByWeightHelper(BSTNode* root, int weight, int higher);
void searchByWeight(const char* country, int weight, int higher, HashNode* hashTable);
void calculateTotalLoadAndValuation(const char* country, HashNode* hashTable);
void displayCheapestAndMostExpensive(const char* country, HashNode* hashTable);
void displayLightestAndHeaviest(const char* country, HashNode* hashTable);
void cleanup(HashNode* hashTable);

int main() {
    HashNode* hashTable = initializeHashTable();

    loadData("couriers.txt", hashTable);

    int choice;
    char country[21];
    int weight;
    int option;

    while (1) {
        printf("\nMenu:\n");
        printf("1. Enter country name and display all the parcels details\n");
        printf("2. Enter country and weight pair to display parcels higher/lower than weight\n");
        printf("3. Display total parcel load and valuation for the country\n");
        printf("4. Display cheapest and most expensive parcel’s details\n");
        printf("5. Display lightest and heaviest parcel for the country\n");
        printf("6. Exit\n");
        printf("Enter your choice: ");
        if (scanf("%d", &choice) != 1) {
            printf("Invalid input, please enter a number.\n");
            while (getchar() != '\n'); // Clear invalid input
            continue;
        }

        switch (choice) {
        case 1:
            printf("Enter country name: ");
            if (scanf("%20s", country) != 1) {
                printf("Invalid input.\n");
                while (getchar() != '\n'); // Clear invalid input
                continue;
            }
            searchByCountry(country, hashTable);
            break;
        case 2:
            printf("Enter country name: ");
            if (scanf("%20s", country) != 1) {
                printf("Invalid input.\n");
                while (getchar() != '\n'); // Clear invalid input
                continue;
            }
            printf("Enter weight: ");
            if (scanf("%d", &weight) != 1) {
                printf("Invalid input.\n");
                while (getchar() != '\n'); // Clear invalid input
                continue;
            }
            printf("1. Higher than weight\n2. Lower than weight\n");
            if (scanf("%d", &option) != 1) {
                printf("Invalid input.\n");
                while (getchar() != '\n'); // Clear invalid input
                continue;
            }
            if (option == 1)
                searchByWeight(country, weight, 1, hashTable);
            else
                searchByWeight(country, weight, 0, hashTable);
            break;
        case 3:
            printf("Enter country name: ");
            if (scanf("%20s", country) != 1) {
                printf("Invalid input.\n");
                while (getchar() != '\n'); // Clear invalid input
                continue;
            }
            calculateTotalLoadAndValuation(country, hashTable);
            break;
        case 4:
            printf("Enter country name: ");
            if (scanf("%20s", country) != 1) {
                printf("Invalid input.\n");
                while (getchar() != '\n'); // Clear invalid input
                continue;
            }
            displayCheapestAndMostExpensive(country, hashTable);
            break;
        case 5:
            printf("Enter country name: ");
            if (scanf("%20s", country) != 1) {
                printf("Invalid input.\n");
                while (getchar() != '\n'); // Clear invalid input
                continue;
            }
            displayLightestAndHeaviest(country, hashTable);
            break;
        case 6:
            cleanup(hashTable);
            free(hashTable);
            return 0;
        default:
            printf("Invalid choice, try again.\n");
        }
    }
}

/* Initialize the hash table */
HashNode* initializeHashTable(void) {
    HashNode* hashTable = (HashNode*)malloc(TABLE_SIZE * sizeof(HashNode));
    if (hashTable == NULL) {
        perror("Unable to allocate memory for hash table");
        exit(1);
    }
    for (int i = 0; i < TABLE_SIZE; ++i) {
        hashTable[i].root = NULL;
    }
    return hashTable;
}

/* Hash function using DJB2 algorithm */
unsigned long computeHash(const char* str) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash % TABLE_SIZE;
}

/* Create a new parcel */
Parcel* createParcel(const char* destination, int weight, float valuation) {
    Parcel* newParcel = (Parcel*)malloc(sizeof(Parcel));
    if (newParcel == NULL) {
        perror("Unable to allocate memory for parcel");
        exit(1);
    }
    newParcel->destination = (char*)malloc(strlen(destination) + 1);
    if (newParcel->destination == NULL) {
        perror("Unable to allocate memory for parcel destination");
        exit(1);
    }
    strcpy(newParcel->destination, destination);
    newParcel->weight = weight;
    newParcel->valuation = valuation;
    newParcel->next = NULL;
    return newParcel;
}

/* Insert parcel into BST */
BSTNode* insertBST(BSTNode* root, Parcel* parcel) {
    if (root == NULL) {
        BSTNode* newNode = (BSTNode*)malloc(sizeof(BSTNode));
        if (newNode == NULL) {
            perror("Unable to allocate memory for BST node");
            exit(1);
        }
        newNode->parcel = parcel;
        newNode->left = newNode->right = NULL;
        return newNode;
    }
    if (parcel->weight < root->parcel->weight)
        root->left = insertBST(root->left, parcel);
    else
        root->right = insertBST(root->right, parcel);
    return root;
}

/* Load data from file into hash table */
void loadData(const char* filename, HashNode* hashTable) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("Unable to open file");
        exit(1);
    }

    char destination[21];
    int weight;
    float valuation;
    while (fscanf(file, "%20[^,],%d,%f\n", destination, &weight, &valuation) != EOF) {
        Parcel* parcel = createParcel(destination, weight, valuation);
        unsigned long index = computeHash(destination);
        hashTable[index].root = insertBST(hashTable[index].root, parcel);
    }
    fclose(file);
}

/* Prints parcels info */
void printParcels(BSTNode* root) {
    if (root != NULL) {
        printParcels(root->left);
        printf("Destination: %s, Weight: %d, Valuation: %.2f\n",
            root->parcel->destination, root->parcel->weight, root->parcel->valuation);
        printParcels(root->right);
    }
}

/* Search parcels by country */
void searchByCountry(const char* country, HashNode* hashTable) {
    unsigned long index = computeHash(country);
    BSTNode* root = hashTable[index].root;
    printParcels(root);
}

/* Display parcels with weight higher or lower than specified */
void searchByWeightHelper(BSTNode* root, int weight, int higher) {
    if (root == NULL) {
        return;
    }
    searchByWeightHelper(root->left, weight, higher);
    if ((higher && root->parcel->weight > weight) ||
        (!higher && root->parcel->weight < weight)) {
        printf("Destination: %s, Weight: %d, Valuation: %.2f\n",
            root->parcel->destination, root->parcel->weight, root->parcel->valuation);
    }
    searchByWeightHelper(root->right, weight, higher);
}

/* Main function to search parcels by weight */
void searchByWeight(const char* country, int weight, int higher, HashNode* hashTable) {
    unsigned long index = computeHash(country);
    BSTNode* root = hashTable[index].root;
    searchByWeightHelper(root, weight, higher);
}

/* Calculate total parcel load and valuation for a country */
void calculateTotalLoadAndValuation(const char* country, HashNode* hashTable) {
    unsigned long index = computeHash(country);
    BSTNode* root = hashTable[index].root;
    int totalWeight = 0;
    float totalValuation = 0.0;

    // Traverse the BST and accumulate weights and valuations
    traverseBST(root, totalWeight, totalValuation);

    printf("Total Load: %d grams, Total Valuation: $%.2f\n", totalWeight, totalValuation);
}

/* Display the cheapest and most expensive parcels */
void displayCheapestAndMostExpensive(const char* country, HashNode* hashTable) {
    unsigned long index = computeHash(country);
    BSTNode* root = hashTable[index].root;
    if (root == NULL) {
        printf("No parcels found for country %s\n", country);
        return;
    }
    Parcel* cheapest = root->parcel;
    Parcel* mostExpensive = root->parcel;
    while (root != NULL) {
        if (root->parcel->valuation < cheapest->valuation)
            cheapest = root->parcel;
        if (root->parcel->valuation > mostExpensive->valuation)
            mostExpensive = root->parcel;
        root = root->right;
    }
    printf("Cheapest Parcel - Destination: %s, Weight: %d, Valuation: %.2f\n",
        cheapest->destination, cheapest->weight, cheapest->valuation);
    printf("Most Expensive Parcel - Destination: %s, Weight: %d, Valuation: %.2f\n",
        mostExpensive->destination, mostExpensive->weight, mostExpensive->valuation);
}

/* Display the lightest and heaviest parcels */
void displayLightestAndHeaviest(const char* country, HashNode* hashTable) {
    unsigned long index = computeHash(country);
    BSTNode* root = hashTable[index].root;
    BSTNode* rootCopy = hashTable[index].root;
    if (root == NULL) {
        printf("No parcels found for country %s\n", country);
        return;
    }
    while (root->left != NULL) {
        root = root->left;
    }
    printf("Lightest Parcel - Destination: %s, Weight: %d, Valuation: %.2f\n",
        root->parcel->destination, root->parcel->weight, root->parcel->valuation);
    root = rootCopy;
    while (root->right != NULL) {
        root = root->right;
    }

    printf("Lightest Parcel - Destination: %s, Weight: %d, Valuation: %.2f\n",
        root->parcel->destination, root->parcel->weight, root->parcel->valuation);
}

/* Cleanup memory */
void cleanup(HashNode* hashTable) {
    for (int i = 0; i < TABLE_SIZE; ++i) {
        BSTNode* root = hashTable[i].root;
        while (root != NULL) {
            BSTNode* temp = root;
            root = root->right;
            free(temp->parcel->destination);
            free(temp->parcel);
            free(temp);
        }
    }
}

void traverseBST(BSTNode* node, int& totalWeight, float& totalValuation) {
    if (node == NULL) {
        return;
    }
    // Traverse the left subtree
    traverseBST(node->left, totalWeight, totalValuation);
    // Visit the current node
    totalWeight += node->parcel->weight;
    totalValuation += node->parcel->valuation;
    // Traverse the right subtree
    traverseBST(node->right, totalWeight, totalValuation);
}