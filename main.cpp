#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <queue>
#include <cstdint>
#include <stdexcept>
#include <unordered_map>

using std::cerr;
using std::ifstream;
using std::ofstream;
using std::runtime_error;
using std::string;

struct ListNode
{
    ListNode *prev = nullptr;
    ListNode *next = nullptr;
    ListNode *rand = nullptr;
    string data;
};

// Освобождение памяти всего списка
void FreeList(ListNode *head)
{
    while (head)
    {
        ListNode *next = head->next;
        delete head;
        head = next;
    }
}

// Сериализация списка в бинарный файл
void Serialize(ListNode *head)
{
    ofstream out("outlet.out", std::ios::binary);
    if (!out)
    {
        throw runtime_error("Cannot open outlet.out");
    }

    // Собираем узлы в вектор для индексации
    std::vector<ListNode *> nodes;
    for (ListNode *cur = head; cur != nullptr; cur = cur->next)
    {
        nodes.push_back(cur);
    }
    uint32_t nodeCount = nodes.size();
    out.write((const char *)(&nodeCount), sizeof(nodeCount));

    // Строим отображение указатель -> индекс для быстрого поиска rand
    std::unordered_map<ListNode *, uint32_t> indexMap;
    for (uint32_t i = 0; i < nodeCount; ++i)
    {
        indexMap[nodes[i]] = i;
    }

    for (uint32_t i = 0; i < nodeCount; ++i)
    {
        ListNode *pNode = nodes[i];
        // Длина строки
        uint32_t strLen = pNode->data.size();
        out.write((const char *)(&strLen), sizeof(strLen));
        // Данные
        out.write(pNode->data.c_str(), strLen);
        // Индекс rand
        int32_t randInd = (pNode->rand) ? static_cast<int32_t>(indexMap[pNode->rand]) : -1;
        out.write((const char *)(&randInd), sizeof(randInd));
    }
}

// Десериализация
void Deserialize(ListNode *&head)
{
    ifstream in("outlet.out", std::ios::binary);
    if (!in)
    {
        throw runtime_error("Cannot open outlet.out");
    }

    // чтение количества узлов списка
    uint32_t nodeCount = 0;
    in.read((char *)(&nodeCount), sizeof(nodeCount));
    if (!in)
        throw runtime_error("Failed to read node count");

    // формирование списка
    if (nodeCount == 0)
    {
        head = nullptr;
        return;
    }

    std::vector<ListNode *> nodes(nodeCount);
    for (uint32_t i = 0; i < nodeCount; ++i)
    {
        if (i == 0)
        {
            nodes[0] = new ListNode;
            head = nodes[0];
        }
        else
        {
            nodes[i] = new ListNode;
            nodes[i]->prev = nodes[i - 1];
            nodes[i - 1]->next = nodes[i];
        }
    }

    // заполнение списка
    for (uint32_t i = 0; i < nodeCount; ++i)
    {
        // чтение длины строки данных
        uint32_t len = 0;
        in.read((char *)(&len), sizeof(len));
        if (!in)
            throw runtime_error("Failed to read string length");

        // чтение строки данных
        string data;
        data.resize(len);
        in.read(&data[0], len);
        if (!in)
            throw runtime_error("Failed to read string data");
        nodes[i]->data = data;

        // чтение rand
        int32_t randInd = 0;
        in.read((char *)(&randInd), sizeof(randInd));
        if (!in)
            throw runtime_error("Failed to read rand index");

        if (randInd < -1 || randInd >= (int32_t)(nodeCount))
            throw runtime_error("Invalid rand index");
        if (randInd != -1)
        {
            nodes[i]->rand = nodes[randInd];
        }
    }
}

// Парсер строки
int strParser(string &str)
{
    if (!str.empty() && str.back() == '\r')
    {
        str.pop_back();
    }
    size_t sep = str.find_last_of(';');
    if (sep == string::npos)
    {
        throw runtime_error("Invalid line \"" + str + "\": no separator ';'");
    }
    string rand_part = str.substr(sep + 1);
    str = str.substr(0, sep);
    try
    {
        return std::stoi(rand_part);
    }
    catch (const std::exception &)
    {
        throw runtime_error("Invalid rand index in line: " + rand_part);
    }
}

int main()
{
    ListNode *head = nullptr, *deHead = nullptr;
    try
    {

        ifstream in("inlet.in");
        if (!in.is_open())
        {
            throw runtime_error("Cannot open inlet.in: this file must be located in same directory.");
        }

        ListNode *pNode = nullptr;
        uint32_t listNodeCount = 0;
        std::queue<int> randIndexes;
        std::vector<ListNode *> nodes;

        string str;
        while (listNodeCount < 1000000 && std::getline(in, str))
        {
            randIndexes.push(strParser(str));
            if (listNodeCount == 0)
            {
                head = new ListNode;
                head->data = str;
                pNode = head;
            }
            else
            {
                pNode->next = new ListNode;
                pNode->next->prev = pNode;
                pNode = pNode->next;
                pNode->data = str;
            }
            nodes.push_back(pNode);
            listNodeCount++;
        }
        in.close();

        pNode = head;

        while (!randIndexes.empty())
        {
            int ind = randIndexes.front();
            randIndexes.pop();
            if (ind < -1)
                throw runtime_error("Incorrect rand-code: rand-code is less than -1.");
            else if (ind >= static_cast<int>(listNodeCount))
                throw runtime_error("Incorrect rand-code: rand-code is greater than or equal to the number of nodes in the list.");
            else if (ind != -1)
            {
                pNode->rand = nodes[ind];
            }
            pNode = pNode->next;
        }

        // Сериализация
        Serialize(head);
        std::cout << "Serialization completed successfully." << std::endl;

        // Десериализация
        Deserialize(deHead);
        ListNode *pNodeDe = deHead;
        pNode = head;

        while (pNode != nullptr && pNodeDe != nullptr)
        {
            if (pNode->data != pNodeDe->data)
                throw runtime_error("Deserialization completed failed.");
            if ((pNode->rand == nullptr && pNodeDe->rand != nullptr) || (pNode->rand != nullptr && pNodeDe->rand == nullptr))
                throw runtime_error("Deserialization completed failed: rand-code is incorrect.");
            else if (pNode->rand != nullptr && pNodeDe->rand != nullptr && pNode->rand->data != pNodeDe->rand->data)
                throw runtime_error("Deserialization completed failed: rand-code is incorrect.");
            pNode = pNode->next;
            pNodeDe = pNodeDe->next;
        }
        if (pNode != nullptr || pNodeDe != nullptr)
            throw runtime_error("Deserialization completed failed: the number of nodes in the list is incorrect.");
        std::cout << "Deserialization completed successfully." << std::endl;

        
    }
    catch (const std::exception &e)
    {
        // Освобождение памяти
        FreeList(head);
        FreeList(deHead);
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    // Освобождение памяти
    FreeList(head);
    FreeList(deHead);
    return 0;
}