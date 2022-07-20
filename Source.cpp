#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <strsafe.h>

#include <chrono>
#include <cmath>

#define MAX_TREE_HT 50

struct MinHNode {
  unsigned char item;
  unsigned int freq;
  MinHNode *left, *right;
};

struct MinHeap {
  unsigned int size;
  unsigned int capacity;
  MinHNode** array;
};

MinHNode* newNode(unsigned char item, unsigned int freq) {
  MinHNode* temp = new MinHNode;

  temp->left = temp->right = NULL;
  temp->item = item;
  temp->freq = freq;

  return temp;
}

MinHeap* createMinH(unsigned int capacity) {
  MinHeap* minHeap = new MinHeap;

  minHeap->size = 0;
  minHeap->capacity = capacity;
  minHeap->array = new MinHNode*[minHeap->capacity];
  return minHeap;
}

void swapMinHNode(MinHNode*& a, MinHNode*& b) {
  MinHNode* t = a;
  a = b;
  b = t;
}

void minHeapify(MinHeap* minHeap, unsigned int idx) {
  unsigned int smallest = idx;
  unsigned int left = 2 * idx + 1;
  unsigned int right = 2 * idx + 2;

  if (left < minHeap->size &&
      minHeap->array[left]->freq < minHeap->array[smallest]->freq)
    smallest = left;

  if (right < minHeap->size &&
      minHeap->array[right]->freq < minHeap->array[smallest]->freq)
    smallest = right;

  if (smallest != idx) {
    swapMinHNode(minHeap->array[smallest], minHeap->array[idx]);
    minHeapify(minHeap, smallest);
  }
}

unsigned int checkSizeOne(MinHeap* minHeap) { return (minHeap->size == 1); }

MinHNode* extractMin(MinHeap* minHeap) {
  MinHNode* temp = minHeap->array[0];
  minHeap->array[0] = minHeap->array[minHeap->size - 1];

  --minHeap->size;
  minHeapify(minHeap, 0);

  return temp;
}

// Insertion function
void insertMinHeap(MinHeap* minHeap, MinHNode* minHeapNode) {
  ++minHeap->size;
  int i = minHeap->size - 1;

  while (i && minHeapNode->freq < minHeap->array[(i - 1) / 2]->freq) {
    minHeap->array[i] = minHeap->array[(i - 1) / 2];
    i = (i - 1) / 2;
  }
  minHeap->array[i] = minHeapNode;
}

void buildMinHeap(MinHeap* minHeap) {
  int n = minHeap->size - 1;
  int i;

  for (i = (n - 1) / 2; i >= 0; --i) minHeapify(minHeap, i);
}

int isLeaf(MinHNode* root) { return !(root->left) && !(root->right); }

MinHeap* createAndBuildMinHeap(unsigned char item[], unsigned int freq[],
                               unsigned int size) {
  MinHeap* minHeap = createMinH(size);

  for (int i = 0; i < size; ++i) minHeap->array[i] = newNode(item[i], freq[i]);

  minHeap->size = size;
  buildMinHeap(minHeap);

  return minHeap;
}

MinHNode* buildHuffmanTree(unsigned char item[], unsigned int freq[],
                           unsigned int size) {
  MinHNode *left, *right, *top;
  MinHeap* minHeap = createAndBuildMinHeap(item, freq, size);

  while (!checkSizeOne(minHeap)) {
    left = extractMin(minHeap);
    right = extractMin(minHeap);

    top = newNode('$', left->freq + right->freq);

    top->left = left;
    top->right = right;

    insertMinHeap(minHeap, top);
  }
  return extractMin(minHeap);
}

// Print the array
void printArray(int arr[], int n) {
  for (int i = 0; i < n; ++i) printf("%d", arr[i]);
  printf("\n");
}

void SetHCodes(struct MinHNode* root, int arr[], int top, uint64_t* charCode) {
  if (root->left) {
    arr[top] = 0;
    SetHCodes(root->left, arr, top + 1, charCode);
  }
  if (root->right) {
    arr[top] = 1;
    SetHCodes(root->right, arr, top + 1, charCode);
  }
  if (isLeaf(root)) {
    for (int i = 0; i < top; ++i)
      charCode[root->item] |= (uint64_t)arr[i] << (top - i - 1);
    charCode[root->item] |= (uint64_t)top << 56;
  }
}

void printHCodes(struct MinHNode* root, int arr[], int top,
                 unsigned int freqTotal, float& entropy, float& codeLength) {
  if (root->left) {
    arr[top] = 0;
    printHCodes(root->left, arr, top + 1, freqTotal, entropy, codeLength);
  }
  if (root->right) {
    arr[top] = 1;
    printHCodes(root->right, arr, top + 1, freqTotal, entropy, codeLength);
  }
  if (isLeaf(root)) {
    float prob = root->freq / (float)freqTotal;
    printf("  %c   | %f    | ", root->item, prob);
    entropy += prob * log2f(1 / prob);
    codeLength += prob * top;
    printArray(arr, top);
  }
}

// Wrapper function
MinHNode* HuffmanCodes(unsigned char item[], unsigned int freq[],
                       unsigned int size) {
  MinHNode* root = buildHuffmanTree(item, freq, size);

  int arr[MAX_TREE_HT], top = 0;
  float entropy = 0, codeLength = 0;
  unsigned int freqTotal = 0;
  for (int i = 0; i < size; ++i) freqTotal += freq[i];

  printHCodes(root, arr, top, freqTotal, entropy, codeLength);
  printf("\nCode Length = %f\n", codeLength);
  printf("Entropy= %f\n", entropy);
  printf("Efficiency= %f\n\n", entropy / codeLength * 100.f);
  return root;
}

int main(int arc, char** argv) {
  HANDLE file = 0;
  LARGE_INTEGER fileSize;
  const char* srcName = "src.txt";
  const char* encodedName = "src-encoded.txt";
  const char* decodedName = "src-decoded.txt";

  unsigned char* srcData;
  unsigned char* encodedData;
  unsigned char* decodedData;
  unsigned int* charFreq;
  uint64_t* charCode;
  auto start = std::chrono::steady_clock::now();

  const wchar_t* fileName = L"src.txt";
  file = CreateFile(fileName, GENERIC_READ, 0, NULL, OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL, NULL);
  if (file == INVALID_HANDLE_VALUE) {
    printf("Failed to open file %ws\n", fileName);
    return 1;
  }

  if (!GetFileSizeEx(file, &fileSize)) {
    printf("Failed to read the file size\n");
    return 1;
  }
  CloseHandle(file);

  if (!(srcData = new unsigned char[fileSize.QuadPart])) {
    printf("Failed to allocate memory for source data");
    return 1;
  }

  if (!(charFreq = (unsigned int*)calloc(1, sizeof(unsigned int) * 256))) {
    printf("Failed to allocate memory for character frequency array");
    return 1;
  }
  if (!(charCode = (uint64_t*)calloc(1, sizeof(uint64_t) * 256))) {
    printf("Failed to allocate memory for character frequency array");
    return 1;
  }

  FILE* hSrc;
  fopen_s(&hSrc, srcName, "rb");
  if (!hSrc) {
    printf("Failed to open the file %s", srcName);
    return 1;
  }

  fread_s(srcData, fileSize.QuadPart, fileSize.QuadPart, 1, hSrc);
  for (uint32_t i = 0; i < fileSize.QuadPart; ++i) charFreq[srcData[i]] += 1;

  int validCharSize = 0;
  for (uint32_t i = 0; i < 256; ++i) {
    if (charFreq[i] == 0) continue;
    validCharSize++;
  }

  unsigned char* arr = new unsigned char[validCharSize];
  unsigned int* freq = new unsigned int[validCharSize];
  {
    int iter = 0;
    for (int i = 0; i < 256; ++i) {
      if (charFreq[i] == 0) continue;
      arr[iter] = (unsigned char)i;
      freq[iter] = charFreq[i];
      iter++;
    }
  }

  printf(" Char | Probability | Huffman code ");
  printf("\n--------------------\n");
  MinHNode* root = HuffmanCodes(arr, freq, validCharSize);

  // Start Encoding
  printf("Start Encoding\n");
  auto cur = std::chrono::steady_clock::now();

  if (!(encodedData = new unsigned char[fileSize.QuadPart])) {
    printf("Failed to allocate memory for source data");
    return 1;
  }
  memset(encodedData, 0, fileSize.QuadPart);

  {
    int arr[MAX_TREE_HT], top = 0;
    SetHCodes(root, arr, top, charCode);
  }

  int encodedSize = 0;
  {
    int bitTrack = 0;
    for (uint32_t iter = 0; iter < fileSize.QuadPart; iter++) {
      uint64_t temp = charCode[srcData[iter]];
      uint8_t codeLen = (temp & (0xff00000000000000)) >> 56;
      for (int i = 0; i < codeLen; ++i) {
        encodedData[encodedSize] |= (temp >> (codeLen - i - 1) & 0x01)
                                    << (7 - (bitTrack));
        bitTrack++;
        if (bitTrack == 8) {
          bitTrack = 0;
          encodedSize++;
        }
      }
    }
    if (bitTrack != 0) {
      encodedSize++;
    }
  }

  delete[](srcData);
  fclose(hSrc);

  printf("Compression Ratio = %f\n\n",
         ((float)fileSize.QuadPart - (float)encodedSize) /
             (float)fileSize.QuadPart * 100.f);

  FILE* hEncoded;
  fopen_s(&hEncoded, encodedName, "wb");
  if (!hEncoded) {
    printf("Failed to open the file %s", encodedName);
    return 1;
  }
  fprintf(hEncoded, "%" PRIu64, uint64_t(fileSize.QuadPart));
  fwrite(encodedData, encodedSize, 1, hEncoded);

  // Decoding

  printf("Elapsed Time: %fs\n",
         (std::chrono::duration_cast<std::chrono::nanoseconds>(
              std::chrono::steady_clock::now() - cur)
              .count()) /
             1000000000.f);

  cur = std::chrono::steady_clock::now();
  printf("Start Decoding\n");

  fclose(hEncoded);
  delete[](encodedData);

  fopen_s(&hEncoded, encodedName, "rb");
  if (!hEncoded) {
    printf("Failed to open the file %s", encodedName);
    return 1;
  }

  uint64_t originalSize = 0;
  if (!fscanf_s(hEncoded, "%" PRIu64, &originalSize)) {
    printf("Failed to read size");
    return 1;
  };

  if (!(decodedData = new unsigned char[originalSize])) {
    printf("Failed to allocate memory for source data");
    return 1;
  }
  decodedData[originalSize - 1] = 0;

  if (!(encodedData = new unsigned char[originalSize])) {
    printf("Failed to allocate memory for source data");
    return 1;
  }
  memset(encodedData, 0, originalSize);
  fread_s(encodedData, originalSize, originalSize, 1, hEncoded);

  int decodedSize = 0;
  MinHNode* temp;
  {
    int bitTrack = 0;
    for (int i = 0; i < encodedSize;) {
      temp = root;
      while (!isLeaf(temp)) {
        unsigned char token = (encodedData[i] >> (7 - bitTrack)) & 0x1;
        if (token == 0)
          temp = temp->left;
        else
          temp = temp->right;
        bitTrack++;
        if (bitTrack == 8) {
          bitTrack = 0;
          i++;
        }
      }
      decodedData[decodedSize] = temp->item;
      decodedSize++;
      if (decodedSize >= originalSize) break;
    }
  }

  printf("Elapsed Time: %fs\n",
         (std::chrono::duration_cast<std::chrono::nanoseconds>(
              std::chrono::steady_clock::now() - cur)
              .count()) /
             1000000000.f);
  cur = std::chrono::steady_clock::now();

  printf("Writing Decoded Data\n");

  FILE* hDecoded;
  fopen_s(&hDecoded, decodedName, "wb");
  if (!hDecoded) {
    printf("Failed to open the file %s", decodedName);
    return 1;
  }
  fwrite(decodedData, decodedSize, 1, hDecoded);
  printf("Elapsed Time: %fs\n",
         (std::chrono::duration_cast<std::chrono::nanoseconds>(
              std::chrono::steady_clock::now() - cur)
              .count()) /
             1000000000.f);

  fclose(hEncoded);
  fclose(hDecoded);
  delete[](decodedData);
  free(charFreq);
  free(charCode);
  delete[](arr);
  delete[](freq);
  printf("\nTotal Elapsed Time: %fs\n",
         (std::chrono::duration_cast<std::chrono::nanoseconds>(
              std::chrono::steady_clock::now() - start)
              .count()) /
             1000000000.f);
}
