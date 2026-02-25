#include <iostream>
#include <fstream>
#include <string>
#include <queue>
#include <cstdint>
// #include <stdexcept>
#include <unordered_map>

using std::cerr;
using std::ifstream;
using std::ofstream;
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
    ofstream out("outfile.out", std::ios::binary);
    if (!out)
    {
        throw std::runtime_error("Cannot open outfile.out");
    }

    // Собираем узлы в вектор для индексации
    std::vector<ListNode *> nodes;
    for (ListNode *cur = head; cur != nullptr; cur = cur->next)
    {
        nodes.push_back(cur);
    }
    uint32_t nodeCount = nodes.size();
    out.write((const char *)(&nodeCount), sizeof(int));

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
void Deserialize(ListNode *head)
{
    std::ifstream in("outfile.out", std::ios::binary);
    if (!in)
    {
        throw std::runtime_error("Cannot open outfile.out");
    }

    // чтение количества узлов списка
    uint32_t nodeCount = 0;
    in.read((char *)(&nodeCount), sizeof(nodeCount));
    if (!in)
        throw std::runtime_error("Failed to read node count");

    // формирование списка
    head = new ListNode;
    ListNode *pNode = head;
    std::unordered_map<uint32_t, ListNode *> indexMap;
    indexMap[0] = head;
    for (uint32_t i = 1; i < nodeCount; ++i)
    {
        pNode->next = new ListNode;
        pNode->next->prev = pNode;
        pNode = pNode->next;
        indexMap[i] = pNode;
    }

    pNode = head;
    // заполнение списка
    for (uint32_t i = 0; i < nodeCount; ++i)
    {
        // чтение длины строки данных
        uint32_t len = 0;
        in.read((char *)(&len), sizeof(len));
        if (!in)
            throw std::runtime_error("Failed to read string length");

        // чтение строки данных
        string data;
        data.resize(len);
        in.read(&data[0], len);
        if (!in)
            throw std::runtime_error("Failed to read string data");
        pNode->data = data;

        // чтение rand
        int32_t randInd = 0;
        in.read((char *)(&randInd), sizeof(randInd));
        if (!in)
            throw std::runtime_error("Failed to read rand index");

        if (randInd < -1 || randInd >= static_cast<int32_t>(nodeCount))
            throw std::runtime_error("Invalid rand index");
        if (randInd != -1)
        {
            pNode->rand = indexMap[randInd];
        }
        pNode = pNode->next;
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
        throw std::runtime_error("Invalid line \"" + str + "\": no separator ';'");
    }
    string data_part = str.substr(0, sep);
    string rand_part = str.substr(sep + 1);
    try
    {
        return std::stoi(rand_part);
    }
    catch (const std::exception &)
    {
        throw std::runtime_error("Invalid rand index in line: " + rand_part);
    }
}

int main()
{
    std::ifstream in("inlet.in");

    try
    {
        const string output_filename = "outlet.out";

        if (!in.is_open())
        {
            throw std::runtime_error("Cannot open inlet.in: this file must be located in same directory.");
        }

        ListNode *head = new ListNode;
        ListNode *pNode = head;
        uint32_t listNodeCount = 0;
        std::queue<int> randIndexes;

        string str;
        while (listNodeCount < 1000000 && std::getline(in, str))
        {
            randIndexes.push(strParser(str));
            if (listNodeCount == 0)
            {
                head->data = str;
            }
            else
            {
                pNode->next = new ListNode;
                pNode->next->prev = pNode;
                pNode = pNode->next;
                pNode->data = str;
            }
            listNodeCount++;
        }

        pNode = head;

        while (!randIndexes.empty())
        {
            ListNode *pRandNode = head;
            int ind = randIndexes.front();
            randIndexes.pop();
            if (ind == -1)
                continue;
            for (; ind != 0 && pRandNode != nullptr; ind--)
                pRandNode = pRandNode->next;
            pNode->rand = pRandNode;
            pNode = pNode->next;
        }

        // Сериализация
        Serialize(head);

        // Освобождение памяти
        FreeList(head);

        std::cout << "Serialization completed successfully." << std::endl;
        return 0;
    }
    catch (const std::exception &e)
    {
        in.close();
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}