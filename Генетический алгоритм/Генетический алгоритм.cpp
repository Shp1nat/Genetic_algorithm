#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
using namespace std;
random_device rd; //random
mt19937 gen(rd()); //random

void printVector(const vector<int>& vec) { //печать вектора
    for (auto el : vec) {
        cout << el << " ";
    }
    cout << endl;
}
void randomShuffle(vector<int>& vec) { //функция рандомного перемешивания вектора
    vector<int> buffer;
    vector<int> newVec;
    buffer.reserve(vec.size());
    newVec.reserve(vec.size());
    for (int i = 0; i < vec.size(); ++i) {
        buffer.push_back(i);
    }
    for (int i = (int)vec.size() - 1; i >= 0; --i) {
        uniform_int_distribution<> makeRandom(0, i);
        int index = makeRandom(gen);
        newVec.push_back(vec[buffer[index]]);
        buffer.erase(buffer.begin() + index);
    }
    vec = newVec;
}
vector<vector<int>> makeMatrix() { //функция изготовления маршрутов (возвращает матрицу маршрутов)
    int countOfPoints;
    cout << "Enter the number of points: ";
    cin >> countOfPoints;
    vector<vector<int>> matrixOfWays(countOfPoints, vector<int>(countOfPoints));
    for (int i = 0; i < countOfPoints; ++i) {
        for (int j = i + 1; j < countOfPoints; ++j) {
            int distance;
            cout << "Enter the distance between points " << i + 1 << " and " << j + 1 <<
                ": ";
            cin >> distance;
            matrixOfWays[i][j] = distance;
            matrixOfWays[j][i] = distance;
        }
    }
    cout << "Your matrix is: " << endl;
    for (const auto& el1 : matrixOfWays) {
        printVector(el1);
    }
    return matrixOfWays;
}
struct Individual { //структура особи (имеет маршрут, его длину, процент мутаций)
    Individual() { //конструктор по умолчанию (вектор пуст, длина 0, процент мутаций 0)
        permutation.clear();
        fitness = 0;
        mutationPercent = 0;
    }
    Individual(const vector<int>& permutation, int fitness, int mutationPercent) { //конструктор с параметрами маршрута, его длины, процента мутаций
        this->permutation = permutation;
        this->fitness = fitness;
        this->mutationPercent = mutationPercent;
    }
    vector<int> permutation; //маршрут
    int fitness; //длина маршрута
    int mutationPercent;
};
int calculateFitness(const vector<int>& permutation, const vector<vector<int>> distanceMatrix) { //функция для расчета длины маршрута
    int fitness = 0;
    for (int i = 0; i < permutation.size() - 1; ++i) {
        fitness += distanceMatrix[permutation[i]][permutation[i + 1]];
    }
    fitness += distanceMatrix[permutation[permutation.size() - 1]][permutation[0]];
    return fitness;
}
Individual selection(const vector<Individual>& population) { //выбор рандомной особи из популяции
    mt19937 generator(rd());
    discrete_distribution<> distribution(population.size(), 0, (double)population.size() - 1, //описываем шанс выбора в рандоме; чем дальше элемент тем меньше шанс на его выбор
        [](double x) { return 1.0 / (x + 1.0); }); //например, 5-ый элемент имеет вес 1/6, а 1-ый - 1/2, то есть в 3 раза чаше будет выпадать 1-ый элемент, чем 5-ый
    return population[distribution(generator)];
}
Individual crossover(const Individual& individual1, const Individual& individual2, const vector<vector<int>>& distanceMatrix) { //кроссинговер, путем объединения двух особей возвращаем новую
    uniform_int_distribution<> makeRandomCutPoint(1, (int)distanceMatrix.size() - 1);
    int cutPoint = makeRandomCutPoint(gen);
    vector<int> child1(distanceMatrix.size());
    vector<int> child2(distanceMatrix.size());
    for (int i = 0; i < cutPoint; ++i) { //добавляем детям точки машрута ДО линии разрыва по их родитеям
        child1[i] = individual1.permutation[i];
        child2[i] = individual2.permutation[i];
    }
    int childIndex = cutPoint;
    for (int i = 0; i < distanceMatrix.size(); ++i) {
        if (find(child1.begin(), child1.end(), individual2.permutation[i]) == child1.end()) { //если у ребенка 1 еще нет в маршруте точки из родителя 2, то добавляем после линии разрыва эти точки поочередно
            child1[childIndex++] = individual2.permutation[i];
        }
    }
    childIndex = cutPoint;
    for (int i = 0; i < distanceMatrix.size(); ++i) {
        if (find(child2.begin(), child2.end(), individual1.permutation[i]) == child2.end()) { //если у ребенка 2 еще нет в маршруте точки из родителя 1, то добавляем после линии разрыва эти точки поочередно
            child2[childIndex++] = individual1.permutation[i];
        }
    }
    uniform_int_distribution<> makeRandomChild(0, 1); //описываем рандом от 0 до 1 для выбора ребенка
    uniform_int_distribution<> makeRandomPercent(1, 10); //описываем рандом от 1 до 10 для обозначения процента мутаций
    vector<int> permutation;
    if (makeRandomChild(gen) == 0) { //выбираем случайно какого ребенка вернем
        permutation = child1;
    }
    else {
        permutation = child2;
    }
    Individual child(permutation, calculateFitness(permutation, distanceMatrix), makeRandomPercent(gen)); //возвращаем особь (ребенка)
    return child;
}
void mutation(vector<Individual>& population, const vector<vector<int>> &distanceMatrix) {
    uniform_int_distribution<> makeRandomPercent(1, 100); //описываем рандом от 1 до 100
    uniform_int_distribution<> makeRandomIndex(0, (int)distanceMatrix.size() - 1); //описываем рандом от 1 до количества точек маршрута
    for (int i = 0; i < population.size(); ++i) { //проходимся по всем особям
        if (makeRandomPercent(gen) < population[i].mutationPercent) { // проверяем, мутирует ли наша особь
            swap(population[i].permutation[makeRandomIndex(gen)], population[i].permutation[makeRandomIndex(gen)]); //меняем две рандомные точки внутри маршрута местами
            population[i].fitness = calculateFitness(population[i].permutation, distanceMatrix); //высчитываем для особи новую длину маршрута
        }
    }
}
pair<vector<int>, int> geneticAlgorithm(vector<vector<int>> distanceMatrix, int countOfGenerations, int sizeOfPopulation) {
    vector<Individual> population; //объявляем начальную популяцию
    for (int i = 0; i < sizeOfPopulation; ++i) { //заполняем начальную популяцию
        vector<int> permutation(distanceMatrix.size()); //создаем маршрут
        for (int i = 0; i < permutation.size(); ++i) { //заполням маршрут всеми точками
            permutation[i] = i;
        }
        randomShuffle(permutation); //рандомно перемешиваем маршрут
        uniform_int_distribution<> makeRandomPercent(1, 10); //описываем рандом от 1 до 10 для обозначения процента мутаций
        Individual individual(permutation, calculateFitness(permutation, distanceMatrix), makeRandomPercent(gen)); //создаем новую особь с параметрами: новый перемешанный маршрут, длина этого маршрута, процент мутаций
        population.push_back(individual); //добавляем созданную особь в нашу начальную популяцию
    }
    for (int generation = 0; generation < countOfGenerations; ++generation) { //инициализируем новые поколения
        mutation(population, distanceMatrix); //производим возможные мутации ДО кроссинговера
        vector<Individual> newPopulation; //объявляем новую популяцию (пока пустая)
        for (int i = 0; i < sizeOfPopulation; ++i) { //заполняем новую популяцию
            Individual parent1 = selection(population); //из уже имеющейся популяции выбираем родителя1
            Individual parent2 = selection(population); //из уже имеющейся популяции выбираем родителя2
            Individual child = crossover(parent1, parent2, distanceMatrix); //объединяем гены родителей, получаем новую особь
            newPopulation.push_back(child); //добавляем новую особь в новую популяцию
        }
        population = newPopulation; //по заполнении всей новой популяции новыми особями, делаем новую популяцию текущей
        mutation(population, distanceMatrix); //проводим мутации с новой популяцией ПОСЛЕ кроссинговера
        sort(population.begin(), population.end(), [](const Individual& id1, const Individual& id2) { //сортируем новую популяцию в порядке возрастания особей по длине маршрута
            return id1.fitness < id2.fitness;
            }); 
    }
    return make_pair(population[0].permutation, calculateFitness(population[0].permutation, distanceMatrix));
}

int main() {
	//Задание параметров алгоритма
    int countOfGenerations = 0, sizeOfPopulation = 0;
    cout << "Enter count of generations (iterations) and size of each population: ";
    cin >> countOfGenerations >> sizeOfPopulation;
    auto result = geneticAlgorithm(makeMatrix(), countOfGenerations, sizeOfPopulation);
    cout << "Best path: ";
    for (const auto& el : result.first) {
        cout << el + 1 << " ";
    }
    cout << endl;
    cout << "Best distance: " << result.second;
	return 0;
}