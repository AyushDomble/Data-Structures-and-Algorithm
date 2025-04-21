#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define ALPHABET_SIZE 26
#define MAX_WORD_LEN 100
#define DICTIONARY_FILE "Dictionary.txt"
#define MAX_SESSION_WORDS 1000
#define STATS_FILE "SearchStats.txt"

// ANSI color macros
#define RED "\x1b[31m"
#define GREEN "\x1b[32m"
#define YELLOW "\x1b[33m"
#define BLUE "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CYAN "\x1b[36m"
#define ORANGE "\x1b[93m"
#define RESET "\x1b[0m"
#define WHITE "\x1b[37m"
#define GREY "\x1b[90m"
#define BOLDYELLOW "\x1b[1;33m"
#define BOLDCYAN "\x1b[1;36m"
#define BOLDRED "\x1b[1;31m"

typedef struct TrieNode
{
    struct TrieNode *children[ALPHABET_SIZE];
    bool isEndOfWord;
} TrieNode;

typedef struct
{
    char word[MAX_WORD_LEN];
    int frequency;
} WordFrequency;

WordFrequency wordFreqList[MAX_SESSION_WORDS];
int wordFreqCount = 0;

TrieNode *createNode()
{
    TrieNode *newNode = (TrieNode *)malloc(sizeof(TrieNode));
    if (newNode)
    {
        newNode->isEndOfWord = false;
        for (int i = 0; i < ALPHABET_SIZE; i++)
            newNode->children[i] = NULL;
    }
    return newNode;
}

void toLowerCase(char *str)
{
    for (int i = 0; str[i]; i++)
        if (str[i] >= 'A' && str[i] <= 'Z')
            str[i] += ('a' - 'A');
}

void insert(TrieNode *root, const char *word)
{
    TrieNode *node = root;
    while (*word)
    {
        int index = *word - 'a';
        if (index < 0 || index >= ALPHABET_SIZE)
            return;
        if (!node->children[index])
            node->children[index] = createNode();
        node = node->children[index];
        word++;
    }
    node->isEndOfWord = true;
}

TrieNode *searchPrefix(TrieNode *root, const char *prefix)
{
    TrieNode *node = root;
    while (*prefix)
    {
        int index = *prefix - 'a';
        if (index < 0 || index >= ALPHABET_SIZE)
            return NULL;
        if (!node->children[index])
            return NULL;
        node = node->children[index];
        prefix++;
    }
    return node;
}

void collectWords(TrieNode *node, char *buffer, int depth)
{
    if (node->isEndOfWord)
    {
        buffer[depth] = '\0';
        printf(CYAN " - %s\n" RESET, buffer);
    }
    for (int i = 0; i < ALPHABET_SIZE; i++)
    {
        if (node->children[i])
        {
            buffer[depth] = i + 'a';
            collectWords(node->children[i], buffer, depth + 1);
        }
    }
}

void updateFrequency(const char *word)
{
    for (int i = 0; i < wordFreqCount; i++)
    {
        if (strcmp(wordFreqList[i].word, word) == 0)
        {
            wordFreqList[i].frequency++;
            return;
        }
    }
    strcpy(wordFreqList[wordFreqCount].word, word);
    wordFreqList[wordFreqCount].frequency = 1;
    wordFreqCount++;
}

void autoSuggest(TrieNode *root, const char *prefix)
{
    TrieNode *node = searchPrefix(root, prefix);
    if (!node)
    {
        printf(BOLDRED "No suggestions found.\n" RESET);
        return;
    }

    updateFrequency(prefix);

    char buffer[MAX_WORD_LEN];
    strcpy(buffer, prefix);
    printf(GREEN "Suggestions:\n" RESET);
    collectWords(node, buffer, strlen(prefix));
}

void loadDictionary(TrieNode *root)
{
    FILE *file = fopen(DICTIONARY_FILE, "r");
    if (!file)
    {
        printf(ORANGE "Warning: Dictionary file not found. Proceeding with an empty trie.\n" RESET);
        return;
    }

    char word[MAX_WORD_LEN];
    while (fgets(word, MAX_WORD_LEN, file))
    {
        word[strcspn(word, "\r\n")] = 0;
        toLowerCase(word);
        insert(root, word);
    }
    fclose(file);
    printf(GREEN "Dictionary loaded successfully!\n" RESET);
}

void saveWordToFile(const char *word)
{
    FILE *file = fopen(DICTIONARY_FILE, "a");
    if (!file)
    {
        printf(BOLDRED "Error opening dictionary file!\n" RESET);
        return;
    }
    fprintf(file, "%s\n", word);
    fclose(file);
}

bool isEmpty(TrieNode *node)
{
    for (int i = 0; i < ALPHABET_SIZE; i++)
        if (node->children[i])
            return false;
    return true;
}

bool deleteWordHelper(TrieNode *node, const char *word)
{
    if (*word)
    {
        int index = *word - 'a';
        if (index < 0 || index >= ALPHABET_SIZE || !node->children[index])
            return false;

        bool shouldDeleteChild = deleteWordHelper(node->children[index], word + 1);

        if (shouldDeleteChild)
        {
            free(node->children[index]);
            node->children[index] = NULL;
            return !node->isEndOfWord && isEmpty(node);
        }
    }
    else if (node->isEndOfWord)
    {
        node->isEndOfWord = false;
        return isEmpty(node);
    }
    return false;
}

bool searchWord(TrieNode *root, const char *word)
{
    TrieNode *node = root;
    while (*word)
    {
        int index = *word - 'a';
        if (index < 0 || index >= ALPHABET_SIZE || !node->children[index])
            return false;
        node = node->children[index];
        word++;
    }
    return node && node->isEndOfWord;
}

void deleteWord(TrieNode *root, const char *word)
{
    if (searchWord(root, word))
    {
        deleteWordHelper(root, word);
        printf(GREEN "Word deleted successfully from Trie.\n" RESET);

        FILE *file = fopen(DICTIONARY_FILE, "r");
        FILE *temp = fopen("temp.txt", "w");
        char line[MAX_WORD_LEN];
        while (fgets(line, sizeof(line), file))
        {
            line[strcspn(line, "\r\n")] = 0;
            if (strcmp(line, word) != 0)
                fprintf(temp, "%s\n", line);
        }
        fclose(file);
        fclose(temp);
        remove(DICTIONARY_FILE);
        rename("temp.txt", DICTIONARY_FILE);
    }
    else
    {
        printf(BOLDRED "Word not found in Trie.\n" RESET);
    }
}

void displayAllWords(TrieNode *root)
{
    char buffer[MAX_WORD_LEN];
    printf(ORANGE "All words in dictionary:\n" RESET);
    collectWords(root, buffer, 0);
}

void findShortestLongestWords(
    TrieNode *node, char *buffer, int depth,
    char **shortestWords, int *shortestLen, int *shortestCount,
    char **longestWords, int *longestLen, int *longestCount)
{

    if (node->isEndOfWord)
    {
        buffer[depth] = '\0';
        int len = strlen(buffer);

        if (*shortestLen == -1 || len < *shortestLen)
        {
            *shortestLen = len;
            *shortestCount = 0;
            strcpy(shortestWords[(*shortestCount)++], buffer);
        }
        else if (len == *shortestLen)
        {
            strcpy(shortestWords[(*shortestCount)++], buffer);
        }

        if (*longestLen == -1 || len > *longestLen)
        {
            *longestLen = len;
            *longestCount = 0;
            strcpy(longestWords[(*longestCount)++], buffer);
        }
        else if (len == *longestLen)
        {
            strcpy(longestWords[(*longestCount)++], buffer);
        }
    }

    for (int i = 0; i < ALPHABET_SIZE; i++)
    {
        if (node->children[i])
        {
            buffer[depth] = i + 'a';
            findShortestLongestWords(
                node->children[i], buffer, depth + 1,
                shortestWords, shortestLen, shortestCount,
                longestWords, longestLen, longestCount);
        }
    }
}

void showShortestLongestWord(TrieNode *root)
{
    char buffer[MAX_WORD_LEN];
    char *shortestWords[100], *longestWords[100];
    for (int i = 0; i < 100; i++)
    {
        shortestWords[i] = malloc(MAX_WORD_LEN);
        longestWords[i] = malloc(MAX_WORD_LEN);
    }

    int shortestLen = -1, longestLen = -1;
    int shortestCount = 0, longestCount = 0;

    findShortestLongestWords(
        root, buffer, 0,
        shortestWords, &shortestLen, &shortestCount,
        longestWords, &longestLen, &longestCount);

    if (shortestCount > 0 && longestCount > 0)
    {
        printf(MAGENTA "Shortest word(s):\n" RESET);
        for (int i = 0; i < shortestCount; i++)
            printf(CYAN " - %s\n" RESET, shortestWords[i]);

        printf(MAGENTA "Longest word(s):\n" RESET);
        for (int i = 0; i < longestCount; i++)
            printf(CYAN " - %s\n" RESET, longestWords[i]);
    }
    else
    {
        printf(BOLDRED "Trie is empty.\n" RESET);
    }

    for (int i = 0; i < 100; i++)
    {
        free(shortestWords[i]);
        free(longestWords[i]);
    }
}

void showRecentlyAdded(char **sessionWords, int sessionWordCount)
{
    if (sessionWordCount == 0)
    {
        printf(YELLOW "No words added during this session.\n" RESET);
        return;
    }
    printf(BOLDYELLOW "Recently Added Words (This Session):\n" RESET);
    for (int i = 0; i < sessionWordCount; i++)
    {
        printf(CYAN " - %s\n" RESET, sessionWords[i]);
    }
}

void showRecentlyDeleted(char **deletedSessionWords, int deletedSessionWordCount)
{
    if (deletedSessionWordCount == 0)
    {
        printf(YELLOW "No words deleted during this session.\n" RESET);
        return;
    }
    printf(BOLDRED "Recently Deleted Words (This Session):\n" RESET);
    for (int i = 0; i < deletedSessionWordCount; i++)
    {
        printf(RED " - %s\n" RESET, deletedSessionWords[i]);
    }
}

void showMostFrequentSearches()
{
    if (wordFreqCount == 0)
    {
        printf(YELLOW "No search history found.\n" RESET);
        return;
    }
    printf(BOLDYELLOW "Most Frequently Searched Words:\n" RESET);
    for (int i = 0; i < wordFreqCount; i++)
    {
        printf(CYAN " - %s (%d times)\n" RESET, wordFreqList[i].word, wordFreqList[i].frequency);
    }
}

void loadSearchStats()
{
    FILE *file = fopen(STATS_FILE, "r");
    if (!file)
        return;

    char word[MAX_WORD_LEN];
    int freq;

    while (fscanf(file, "%s %d", word, &freq) == 2)
    {
        strcpy(wordFreqList[wordFreqCount].word, word);
        wordFreqList[wordFreqCount].frequency = freq;
        wordFreqCount++;
    }

    fclose(file);
}

void saveSearchStats()
{
    FILE *file = fopen(STATS_FILE, "w");
    if (!file)
        return;

    for (int i = 0; i < wordFreqCount; i++)
    {
        fprintf(file, "%s %d\n", wordFreqList[i].word, wordFreqList[i].frequency);
    }

    fclose(file);
}

int main()
{
    TrieNode *root = createNode();
    loadDictionary(root);
    loadSearchStats();

    int choice;
    char word[MAX_WORD_LEN];
    char *sessionWords[MAX_SESSION_WORDS];
    int sessionWordCount = 0;

    char *deletedSessionWords[MAX_SESSION_WORDS];
    int deletedSessionWordCount = 0;

    while (1)
    {
        printf("\n" BOLDCYAN "--- Auto-Suggest System ---\n" RESET);
        printf("\x1b[38;5;208m");
        printf("1. Add a new word\n");
        printf("2. Search by prefix (Auto-suggestions)\n");
        printf("3. Display all words\n");
        printf("4. Show recently added words\n");
        printf("5. Show shortest & longest word\n");
        printf("6. Delete a word\n");
        printf("7. Show recently deleted words\n");
        printf("8. Undo last deleted word\n");
        printf("9. Show most frequently searched words\n");
        printf("10. Exit\n");
        printf(RESET "Enter your choice: ");
        scanf("%d", &choice);
        getchar();

        switch (choice)
        {
        case 1:
            printf(GREY "Enter word to add: " RESET);
            scanf("%s", word);
            toLowerCase(word);
            insert(root, word);
            saveWordToFile(word);
            sessionWords[sessionWordCount] = malloc(strlen(word) + 1);
            strcpy(sessionWords[sessionWordCount++], word);
            printf(GREEN "Word added successfully!\n" RESET);
            break;
        case 2:
            printf(GREY "Enter prefix: " RESET);
            scanf("%s", word);
            toLowerCase(word);
            autoSuggest(root, word);
            break;
        case 3:
            displayAllWords(root);
            break;
        case 4:
            showRecentlyAdded(sessionWords, sessionWordCount);
            break;
        case 5:
            showShortestLongestWord(root);
            break;
        case 6:
            printf(GREY "Enter word to delete: " RESET);
            scanf("%s", word);
            toLowerCase(word);

            if (searchWord(root, word))
            {
                char confirm;
                printf(ORANGE "Are you sure you want to delete \"%s\"? (y/n): " RESET, word);
                getchar();
                scanf("%c", &confirm);
                if (confirm == 'y' || confirm == 'Y')
                {
                    deleteWord(root, word);
                    for (int i = 0; i < sessionWordCount; i++)
                    {
                        if (strcmp(sessionWords[i], word) == 0)
                        {
                            free(sessionWords[i]);
                            for (int j = i; j < sessionWordCount - 1; j++)
                            {
                                sessionWords[j] = sessionWords[j + 1];
                            }
                            sessionWords[--sessionWordCount] = NULL;
                            break;
                        }
                    }
                    deletedSessionWords[deletedSessionWordCount] = malloc(strlen(word) + 1);
                    strcpy(deletedSessionWords[deletedSessionWordCount++], word);
                }
                else
                {
                    printf(YELLOW "Deletion cancelled.\n" RESET);
                }
            }
            else
            {
                printf(BOLDRED "Word not found in Trie.\n" RESET);
            }
            break;
        case 7:
            showRecentlyDeleted(deletedSessionWords, deletedSessionWordCount);
            break;
        case 8:
            if (deletedSessionWordCount == 0)
            {
                printf(YELLOW "No deleted words to undo.\n" RESET);
            }
            else
            {
                char *wordToRestore = deletedSessionWords[--deletedSessionWordCount];
                insert(root, wordToRestore);
                saveWordToFile(wordToRestore);
                sessionWords[sessionWordCount] = malloc(strlen(wordToRestore) + 1);
                strcpy(sessionWords[sessionWordCount++], wordToRestore);
                printf(GREEN "Successfully restored \"%s\" to Trie and Dictionary.\n" RESET, wordToRestore);
                free(deletedSessionWords[deletedSessionWordCount]); // Free pointer from deleted list
                deletedSessionWords[deletedSessionWordCount] = NULL;
            }
            break;
        case 9:
            showMostFrequentSearches();
            break;
        case 10:
            printf(BOLDYELLOW "PROGRAM EXITED SUCCESSFULLY.\n" RESET);
            saveSearchStats();
            for (int i = 0; i < sessionWordCount; i++)
                free(sessionWords[i]);
            for (int i = 0; i < deletedSessionWordCount; i++)
                free(deletedSessionWords[i]);
            return 0;
        default:
            printf(BOLDRED "Invalid choice! Please try again.\n" RESET);
        }
    }
}
