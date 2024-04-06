#include <windows.h>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <algorithm>
#define min(a, b) (((a) < (b)) ? (a) : (b))


// всі розміри знизу зазначені в пікселях

// пікселі можна вважати за координати на вікні, по котрим відмальовуються всі об'єкти у вікні
// координата (0, 0) знаходиться у верхньому лівому куті вікна

const int WINDOW_WIDTH = 800; // ширина вікна
const int WINDOW_HEIGHT = 600; // висота вікна
const int PADDLE_WIDTH = 10; // ширина платформи гравця
const int PADDLE_HEIGHT = 100; // висота платформи гравця
const int BALL_SIZE = 15; // сторона квадратного м'яча
const int WALL_WIDTH = 15; // ширина обмежувальних стінок
const int BAR_HEIGHT = 23; // відстань від верхньої точки вікна до верхньої межі ігрового поля. Застарілий функціонал, прибирати не став 
const int BUTTON_WIDTH = 100; // ширина кнопок
const int BUTTON_HEIGHT = 40; // висота кнопок

const int PADDLE_SPEED = 10; // відстань, яку проходить платформа за один *тік
const int BALL_SPEED = 3; // відстань, котру проходить м'як за один *тік

// *тік - умовно один "кадр", котрий оновлюється раз на g_TimerDelay мілісекунд
const int g_TimerDelay = 16; // затримка між "тіками" (кадрами)

const COLORREF g_BgColor = RGB(222, 231, 249); // Встановлюємо колір фону у форматі RGB



// HWND (Handles to a Window) - тип даних, котрий відповідає за вікно. Тобто ця змінна якби і є самим вікном.
// Посилаючись на неї, ми отримуємо доступ до конктертного вікна

// ВАЖЛИВО ЗАЗНАЧИТИ: Кожна частина додатку, будь то пункт меню чи кнопка, є окремим вікном. Що це означає:
// коли ми запускаємо даний додаток, вінда створює пусте вікно, на котре накладає інші вікна одне на одне.
// Наприклад: при запуску гри, відображається вікно g_hWndMenu, на котре накладаються g_StartButton, g_RulesButton та g_ExitButton, котрі є кнопками меню

HWND g_hWndMenu; // ініціалізація вікон додатку, з назви зрозуміло що є чим 
HWND g_hWndGame;

HWND g_StartButton;
HWND g_RulesButton;
HWND g_ExitButton;


HWND g_PlusButton;
HWND g_MinusButton;

// Тут нема вікон з правилами та результатами гри, оскільки вони є вспливаючими. Тобто вони створюються окремо, а не накладаються на основне як, наприклад, кнопки.
// Робиться це простіше, однією функцією. Описано це трохи нижче.

UINT_PTR g_TimerID; // Ініціалізуємо змінну під таймер. Він прив'язується до конкретного вікна і ця змінна потрібна, щоб можна було звернутися до таймера в будь-якій точці програми і зупинити його

int g_totalScore = 0; // Поточний рахунок гравця
int g_WinningScore = 3; // Рахунок, необхідний набрати для того, щоб перемогти
int g_PlayerY = WINDOW_HEIGHT / 2 - PADDLE_HEIGHT / 2; // встановлення початкової позиції платформи гравця на центр поля
int g_BallX = WINDOW_WIDTH / 2 - BALL_SIZE / 2; // встановлення початкової позиції м'яча на центро поля по осі Х
int g_BallY = WINDOW_HEIGHT / 2 - BALL_SIZE / 2; // встановлення початкової позиції м'яча на центро поля по осі У
int g_BallDirX = 1; // одиничний вектор напрямку м'яча по осі Х. Інвертується відносно 0, коли м'яч б'ється у праву стінку поля або платформу гравця
int g_BallDirY = 1; // аналогічно для осі У. Інвертується, коли м'яч б'ється у верхню або нижню стінку поля



// Ініціалізація функцій в додатку, кожну буду розбирати окремо
// ---------------------------------------------------------------------------------
LRESULT CALLBACK WndProcMenu(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProcGame(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

void CreateMenuButtons(HWND hWnd);
void UpdateGame();
void DrawGame(HDC hdc);
void DrawMenu(HDC hdc);
void MovePlayer(int direction);
void ResetBall();
void SaveResultsToFile();
std::wstring GetResultsFromFile();
// ---------------------------------------------------------------------------------




// Дана функція є точкою входу в програму, звідси починається її виконання
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) // На аргументи сильно не дивись, я тобі це не поясню, так воно в документаціїї і майже ніколи ніхто це не змінює
{
    WNDCLASSEX wcexMenu, wcexGame; // Дві структури, котрі містять деякі важливі параметри вікна, найважливіший з котрих - lpfnWndProc.
    // Цей параметр містить ім'я функції, котра "ловить події" (пояснюю нижче), котрі відбуваються у вікні

    // Що таке події? Це такі собі індикатори того, що щось відбулося у вікні, наприклад вікно було створено, щось було відмальовано у вікні, відбулася якась взаємодія, тощо.
    // Як конкретно тут працюють подіїї поясню нижче, на самих функціях, котрі їх оброблюють

    HBRUSH hb = ::CreateSolidBrush(g_BgColor);

    wcexMenu.cbSize = sizeof(WNDCLASSEX);
    wcexMenu.style = CS_HREDRAW | CS_VREDRAW;
    wcexMenu.lpfnWndProc = WndProcMenu; // Назва функції, котра оброблює події у вікні меню
    wcexMenu.cbClsExtra = 0;
    wcexMenu.cbWndExtra = 0;
    wcexMenu.hInstance = hInstance;
    wcexMenu.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcexMenu.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcexMenu.hbrBackground = hb;

    wcexMenu.lpszMenuName = NULL;
    wcexMenu.lpszClassName = TEXT("MyMenuClass");
    wcexMenu.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    wcexGame.cbSize = sizeof(WNDCLASSEX);
    wcexGame.style = CS_HREDRAW | CS_VREDRAW;
    wcexGame.lpfnWndProc = WndProcGame; // Назва функції, котра оброблює події у вікні гри
    wcexGame.cbClsExtra = 0;
    wcexGame.cbWndExtra = 0;
    wcexGame.hInstance = hInstance;
    wcexGame.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcexGame.hCursor = LoadCursor(NULL, IDC_ARROW);

    wcexGame.hbrBackground = hb;

    wcexGame.lpszMenuName = NULL;
    wcexGame.lpszClassName = TEXT("MyGameClass");
    wcexGame.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    if (!RegisterClassEx(&wcexMenu) || !RegisterClassEx(&wcexGame)) // Перевіряє, чи правильно ініціалізувались струтктури і завершує виконання, якщо ні
    {
        MessageBox(NULL, TEXT("Call to RegisterClassEx failed!"), TEXT("Win32"), MB_OK);
        return 1;
    }

    g_hWndMenu = CreateWindow( // Створюємо саме вікно меню, зазначаючи базові параметри
        TEXT("MyMenuClass"),
        TEXT("Tennis"), // Назва вікна
        WS_OVERLAPPEDWINDOW, // Зазначення деяких властивостей. Погугли, якщо цікаво
        CW_USEDEFAULT, CW_USEDEFAULT,
        WINDOW_WIDTH, WINDOW_HEIGHT, // Оці два ми ініціалізували вище і відповідають вони за ширину і висоту відповідно
        NULL,
        NULL,
        hInstance,
        NULL
    );

    g_hWndGame = CreateWindow( // Все те саме, тільки для вікна гри
        TEXT("MyGameClass"),
        TEXT("Tennis"),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    ShowWindow(g_hWndMenu, nCmdShow); // Оскільки це вхідна функція, спочатку показуємо вікно меню
    UpdateWindow(g_hWndMenu); // Оновлюємо його, щоб все відобразилося корректно. Коли буду пояснювати функцію, котра ловить події, поясню навіщо його оновлювати на старті.



    // Цей шматок коду використовується для обробки повідомлень у циклі та відправлення їх до функції обробки повідомлень
    // -------------------------------------------
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
    // -------------------------------------------
}


void CreateMenuButtons(HWND hWnd) // Функція для встановлення параметрів кнопок у вікні меню
{
    g_StartButton = CreateWindow( // Як я вже казав, кнопки це теж вікна, тож ми використовуємо CreateWindow
        TEXT("BUTTON"),
        TEXT("Start"),
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, // Декілька важливих параметрів, гугли якщо цікаво
        int(WINDOW_WIDTH / 2 - BUTTON_WIDTH / 2), 200, BUTTON_WIDTH, BUTTON_HEIGHT, // Ці чотири параметри відповідають за наступні речі відповідно: позиція кнопки по Х, позиція кнопки по У, ширина в пікселях і висота  
        hWnd,
        (HMENU)1, // Індекс кнопки
        GetModuleHandle(NULL),
        NULL
    );

    // З іншими кнопками все аналогічно

    g_RulesButton = CreateWindow(
        TEXT("BUTTON"),
        TEXT("Rules"),
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        int(WINDOW_WIDTH / 2 - BUTTON_WIDTH / 2), 250, BUTTON_WIDTH, BUTTON_HEIGHT,
        hWnd,
        (HMENU)2,
        GetModuleHandle(NULL),
        NULL
    );

    g_ExitButton = CreateWindow(
        TEXT("BUTTON"),
        TEXT("Exit"),
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        int(WINDOW_WIDTH / 2 - BUTTON_WIDTH / 2), 300, BUTTON_WIDTH, BUTTON_HEIGHT,
        hWnd,
        (HMENU)3,
        GetModuleHandle(NULL),
        NULL
    );

    g_PlusButton = CreateWindow(
        TEXT("BUTTON"),
        TEXT("+"),
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        int(WINDOW_WIDTH / 2 - 35), WINDOW_HEIGHT - 100, 30, 30,
        hWnd,
        (HMENU)4,
        GetModuleHandle(NULL),
        NULL
    );

    g_MinusButton = CreateWindow(
        TEXT("BUTTON"),
        TEXT("-"),
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        int(WINDOW_WIDTH / 2 + 5), WINDOW_HEIGHT - 100, 30, 30,
        hWnd,
        (HMENU)5,
        GetModuleHandle(NULL),
        NULL
    );
}

// Функція обробник подій для вікна меню, викликається, коли у вікні відбувається майже будь-яка дія.
LRESULT CALLBACK WndProcMenu(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) // Аргумент, котрий нас цікавить - UINT message. По суті він містить ідентифікатор події, що відбулася
{
    switch (message) // Щоб дізнатися, яка саме подія відбулася, використовуємо світч-кейс
    {
    case WM_CREATE: // Подія створення вікна
        CreateMenuButtons(hWnd); //Ініціалізуємо кнопки викликаючи описану вище функцію
        break;
    case WM_COMMAND: // Спрацьовує при натисканні кнопок. Тут використовуємо ще один світч-кейс, щоб дізнатися яка саме кнопка була натиснута
        switch (LOWORD(wParam))
        { // Кожна кнопка має свій індекс, котрий визначається при її створенні. Дивись функцію CreateMenuButtons
        case 1: // Дії по кнопці Start
            ShowWindow(g_hWndMenu, SW_HIDE); // Приховуємо вікно меню
            ShowWindow(g_hWndGame, SW_SHOWNORMAL); // і показуємо вікно гри
            SetTimer(g_hWndGame, 1, g_TimerDelay, NULL); // ВАЖЛИВО: З початком гри ми запускаємо таймер, котрий використовується для того, щоб раз в тік можна було оновлювати кадр гри.
            // Кожен тік таймера це теж подія, котра ловиться у функціїї обробки подій вікна гри
            break;
        case 2: // Дії по кнопці Rules
            MessageBox(hWnd, TEXT("Game Tennis: after pressing button [Start], you have to catch a ball with a platform to bounce it off and earn points. To move the platform, use keyboard arrows UP and DOWN. You can adjust the maximum score required to win the game by pressing buttons [+] and [-]."), TEXT("Rules"), MB_OK);
            // Виведення стандартного діалогового вікна з текстом правил
            break;
        case 3: // Дії по кнопці Exit
            PostQuitMessage(0); // Закриває вікно і завершує роботу програми
            break;
        case 4:
            g_WinningScore += 1;
            InvalidateRect(hWnd, NULL, TRUE);
            break;
        case 5:
            if (g_WinningScore > 1) { g_WinningScore -= 1; }
            InvalidateRect(hWnd, NULL, TRUE);
            break;
        }
        break;
    case WM_PAINT: // Детальніше описано в наступній функції
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        DrawMenu(hdc); // Викликаємо відмальовку меню для малювання напису
        EndPaint(hWnd, &ps);
    }
    break;
    case WM_DESTROY:
        PostQuitMessage(0); // З документації: вказує системі, що потік зробив запит на завершення роботи. По-людськи: закриває вікно, коли завершується процес. Простіше: використовується для правильного завершення роботи.
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

LRESULT CALLBACK WndProcGame(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_TIMER: // Тут ловиться кожен тік
        UpdateGame(); // Функція з логікою, котра прораховує всі координати і перевіряє м'яч на "зіткнення з об'єктами"

        // Чому в лапках? Бо нема ніяких об'єктів. Я міг би тут реалізувати систему з клаассом хітбоксу, але це дуже роздуло би код і зробила б його ще більш незрозумілим для тебе
        // через прив'язку логіки до windows.h

        // Що маємо на ділі: все, що малюється на екрані протягом гри просто відображає задані координатами фігури. Вся магія відбувається лише умовно в UpdateGame, де з кожним тіком
        // Перераховуються координати м'яча. Вже після її виконання це все перемальовується на екран. 

        // По-простому: стінки, платформа це просто чорні фігури. М'яч міняє свій напрям перетинаючи уявні межі.


        InvalidateRect(hWnd, NULL, TRUE); // Очищує вікно
        break;
    case WM_KEYDOWN: // Подія натискання клавіши
        switch (wParam) // Світч-кейс для визначення якої саме
        {
        case VK_UP:
            MovePlayer(-PADDLE_SPEED); // Міняє положення платформи, прийчаючи в аргумент швидкість за тік
            break;
        case VK_DOWN:
            MovePlayer(PADDLE_SPEED);
            break;
        }
        break;
    case WM_PAINT: // Подія відмальовування будь-чого у вікні
    {
        // Оцей шматок коду малює всі фігури у вікні
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        DrawGame(hdc);
        EndPaint(hWnd, &ps);
    }
    break;
    case WM_DESTROY: // Аналогічно попередньому
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

void UpdateGame() // Тут вся логіка, викликається кожен тік, проста геометрія, второпаєш
{
    g_BallX += g_BallDirX * BALL_SPEED;
    g_BallY += g_BallDirY * BALL_SPEED;

    if (g_BallY <= BAR_HEIGHT || g_BallY >= WINDOW_HEIGHT - 4 * WALL_WIDTH - BALL_SIZE)
    {
        g_BallDirY = -g_BallDirY;
    }

    if (g_BallX >= WINDOW_WIDTH - 2 * WALL_WIDTH - BALL_SIZE)
    {
        g_BallDirX = -g_BallDirX;
    }

    if (g_totalScore >= g_WinningScore)
    {
        ResetBall();
        std::wstring answer = L"Congratulations! You won the game.\n\nTop Scores:\n\n" + GetResultsFromFile();
        MessageBox(g_hWndGame, answer.c_str(), TEXT("Game Over"), MB_OK);
        g_totalScore = 0;

        return;
    }

    if (g_BallX <= 0)
    {
        ResetBall();
        std::wstring answer = L"Game over. You lost.\n\nTop Scores:\n\n" + GetResultsFromFile();
        MessageBox(g_hWndGame, answer.c_str(), TEXT("Game Over"), MB_OK);
        g_totalScore = 0;

        return;
    }

    if ((g_BallX <= PADDLE_WIDTH && g_BallY >= g_PlayerY && g_BallY <= g_PlayerY + PADDLE_HEIGHT))
    {
        g_BallDirX = -g_BallDirX;
        g_totalScore += 1;
    }
}


// Функція, що відмальовує гру
void DrawGame(HDC hdc)
{
    SetBkColor(hdc, g_BgColor);

    RECT player = { PADDLE_WIDTH, g_PlayerY, PADDLE_WIDTH + PADDLE_WIDTH, g_PlayerY + PADDLE_HEIGHT }; // RECT - тип даних, кторий задає прямокутник, приймає 4 значення по порядку:
    // верхня ліва точка Х, верхня ліва точка У, права нижня точка Х, права нижня точка У
    RECT wall1 = { WINDOW_WIDTH - 2 * WALL_WIDTH, BAR_HEIGHT, WINDOW_WIDTH - WALL_WIDTH, WINDOW_HEIGHT - 3 * WALL_WIDTH };
    RECT wall2 = { 0, WINDOW_HEIGHT - 4 * WALL_WIDTH, WINDOW_WIDTH - WALL_WIDTH, WINDOW_HEIGHT - 3 * WALL_WIDTH };
    RECT wall3 = { 0, BAR_HEIGHT - WALL_WIDTH, WINDOW_WIDTH - WALL_WIDTH, BAR_HEIGHT };
    RECT ball = { g_BallX, g_BallY, g_BallX + BALL_SIZE, g_BallY + BALL_SIZE };

    FillRect(hdc, &player, (HBRUSH)GetStockObject(BLACK_BRUSH)); // Щоб такий прямокутник відобразити, його треба заповнити кольором
    FillRect(hdc, &wall1, (HBRUSH)GetStockObject(BLACK_BRUSH));
    FillRect(hdc, &wall2, (HBRUSH)GetStockObject(BLACK_BRUSH));
    FillRect(hdc, &wall3, (HBRUSH)GetStockObject(BLACK_BRUSH));
    FillRect(hdc, &ball, (HBRUSH)GetStockObject(BLACK_BRUSH));

    std::wstring scoreString = L"Score: " + std::to_wstring(g_totalScore);

    TextOut(hdc, int(WINDOW_WIDTH * 4 / 5), 30, scoreString.c_str(), scoreString.size()); // Функція виводу тексту, приймає вказівник на вікно, координати по Х і У, символьний масив з текстом і довжину цього масиву
}

// Функція, що відмальовує меню, зроблена для виводу назви гри та максимального рахунку
void DrawMenu(HDC hdc)
{
    SetBkColor(hdc, g_BgColor);
    // Створюємо шрифт для великого напису
    HFONT hFont = CreateFont(68, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, TEXT("Arial"));
    HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);

    std::wstring text = L"Tennis";
    int textLength = text.size();
    int x = WINDOW_WIDTH / 2 - 100;
    int y = WINDOW_HEIGHT / 4 - 50;
    TextOut(hdc, x, y, text.c_str(), textLength);

    SelectObject(hdc, hOldFont); // Відновлюємо шрифт до початкового
    DeleteObject(hFont); // Видаляємо створений шрифт

    // Відображаємо максимальний рахунок
    std::wstring scoreText = L"Winning Score: " + std::to_wstring(g_WinningScore);
    TextOut(hdc, WINDOW_WIDTH / 2 - 50, WINDOW_HEIGHT - 135, scoreText.c_str(), scoreText.length());
}

void MovePlayer(int direction) // Рухає платформу гравця у визначених межах
{
    if (g_PlayerY + direction >= BAR_HEIGHT - 5 && g_PlayerY + direction <= WINDOW_HEIGHT - PADDLE_HEIGHT - 4 * WALL_WIDTH)
    {
        g_PlayerY += direction;
    }
}

void ResetBall() // При завершенні гри зупиняє таймер, приховує вікно гри і відображає вікно меню
{
    g_BallX = WINDOW_WIDTH / 2 - BALL_SIZE / 2;
    g_BallY = WINDOW_HEIGHT / 2 - BALL_SIZE / 2;
    g_BallDirX = abs(g_BallDirX);
    KillTimer(g_hWndGame, 1);
    ShowWindow(g_hWndMenu, SW_SHOWNORMAL);
    ShowWindow(g_hWndGame, SW_HIDE);
    SaveResultsToFile();
}

void SaveResultsToFile() // Збереження результатів у файл results.txt
{
    std::ofstream file("results.txt", std::ios::app);  // відкриваємо файл для доповнення
    if (file.is_open())
    {
        file << "Score: " << g_totalScore << std::endl;
        file.close();
    }
}

std::wstring GetResultsFromFile()
{
    std::ifstream file("results.txt");
    std::vector<int> scores;
    std::string line;
    int score;

    // Читання та парсинг файлу
    while (getline(file, line)) {
        if (sscanf_s(line.c_str(), "Score: %d", &score) == 1) {
            scores.push_back(score);
        }
    }

    // Сортування у зворотньому порядку, щоб найбільші значення були на початку
    std::sort(scores.rbegin(), scores.rend());

    // Формування рядка з ТОП 3 результатами
    std::wstring result;
    result += L"TOP 1: ";
    for (int i = 0; i < (((3) < (static_cast<int>(scores.size()))) ? (3) : (static_cast<int>(scores.size()))); ++i) {
        if (i > 0) {
            result += L"\nTOP " + std::to_wstring(i+1) + L": ";
        }
        result += std::to_wstring(scores[i]);
    }

    return result;
}