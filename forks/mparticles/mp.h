//-----------------------------------------------------------------------------
// File: mp.h
//
// Copyright (c) Astralax. All rights reserved.
// Author: Sedov Alexey
//-----------------------------------------------------------------------------

#ifndef MAGIC_PARTICLES
#define MAGIC_PARTICLES

#include <string>
#include "magic.h"

class MP_Emitter;
class MP_Manager;
class MP_Atlas;
class MP_Copy;
class MP_Device_WRAP;
struct MP_POSITION;
struct MP_PTC;

#define MAGIC_INTERPOLATION_DEFAULT			0	// eng: interpolation mode is taken from emitter settings
												// rus: режим интерполяции берется из эмиттера
#define MAGIC_INTERPOLATION_ENABLE			1	// eng: interpolation is always enabled 
												// rus: всегда использовать интерполяцию
#define MAGIC_INTERPOLATION_DISABLE			2	// eng: interpolation is always disabled
												// rus: всегда отключать интерполяцию

#define MAGIC_CHANGE_EMITTER_ONLY			0	// eng: preserve particle positions while changing emitter position or direction 
												// rus: при изменении позиции или направления эмиттера частицы остаются на прежнем месте
#define MAGIC_CHANGE_EMITTER_AND_PARTICLES	1	// eng: move all the special effect when changing emitter position or direction 
												// rus: при изменении позиции или направления эмиттера весь спецэффект перемещается целиком
#define MAGIC_CHANGE_EMITTER_DEFAULT		2	// eng: for each special effect setting specified in editor is used 
												// rus: не изменять настройку по умолчанию

// eng: Getting identity matrix 
// rus: Возвращает единичную матрицу
extern void MatrixIdentity(MAGIC_MATRIX* m);

// eng: Calculation of perspective projection matrix (analogue of D3DXMatrixPerspectiveFovLH from Direct3D) 
// rus: Расчет матрицы перспективной проекции (аналог D3DXMatrixPerspectiveFovLH из Direct3D)
extern void MatrixPerspectiveFovLH(MAGIC_MATRIX* m, float fovy, float aspect, float zn, float zf);

// eng: Calculation of orthogonal projection (analogue of D3DXMatrixOrthoLH from Direct3D) 
// rus: Расчет матрицы ортогональной проекции (аналог D3DXMatrixOrthoLH из Direct3D)
extern void MatrixOrthoLH(MAGIC_MATRIX* m, float width, float height, float zn, float zf);

// eng: Calculation of view matrix (analogue of D3DXMatrixLookAtLH from Direct3D) 
// rus: Расчет матрицы вида (аналог D3DXMatrixLookAtLH из Direct3D)
extern void MatrixLookAtLH(MAGIC_MATRIX* m, MP_POSITION* peye, MP_POSITION* pat, MP_POSITION* pup);

class MP_Platform
{
protected:
	std::string ptc_path;
	std::string texture_path;
	std::string temp_path;

	std::string wchar_t8;

public:
	static MP_Manager* MP;

public:
	MP_Platform();
	virtual ~MP_Platform();

	// eng: Returns time in milliseconds
	// rus: Возвращает время в милисекундах
	virtual unsigned long GetTick()=0;

	// eng: Returns path to folder with emitters
	// rus: Возвращает путь к папке с ptc-файлами
	virtual const char* GetPathToPTC()=0;

	// eng: Returns path to folder with textures
	// rus: Возвращает путь к папке с текстурами
	virtual const char* GetPathToTexture()=0;

	// eng: Returns path to folder which could be used by wrapper to store temporary files
	// rus: Возвращает путь к временной папке
	virtual const char* GetPathToTemp()=0;

	// eng: Finds the first ptc-file in emitters folder
	// rus: Поиск первого ptc-файла в папке с эмиттерами
	virtual const char* GetFirstFile(){return NULL;}

	// eng: Finds the next ptc-file in emitters folder
	// rus: Поиск очередного ptc-файла в папке с эмиттерами
	virtual const char* GetNextFile(){return NULL;}

	// eng: Converts wchar_t string into utf8
	// rus: Конвертирует строку типа wchar_t в строку типа utf8
	virtual const char* wchar_to_utf8(const wchar_t* str);

	// eng: Converts utf8 string into wchar_t
	// rus: Конвертирует строку типа utf8 в строку типа wchar_t
	virtual const wchar_t* utf8_to_wchar(const char* str);

	// eng: Opens ptc-file
	// rus: Открывает ptc-файл
	virtual HM_FILE OpenPTC(const char* ptc_file);

	// Deleting of file (analogue remove())
	// Удаление файла (аналог remove())
	virtual int RemoveFile(const char* file)=0;
};

// eng: Class that is used as storage for Magic Particles emitters
// rus: Класс, который является хранилищем эмиттеров Magic Particles
class MP_Manager
{
protected:
	MP_Platform* platform;	// eng: object implementing OS specifics
							// rus: объект работающий со спецификой OS

	// eng: number of loaded emitters
	// rus: количество загруженных эмиттеров
	int k_emitter;
	int max_emitter;

	// eng: list of loaded emitters
	// rus: список загруженных эмиттеров
	MP_Emitter** m_emitter;
	HM_EMITTER* m_descriptor;

	// eng: number of loaded atlases
	// rus: количество загруженных атласов
	int k_atlas;

	// eng: list of loaded atlases
	// rus: список загруженных атласов
	MP_Atlas** m_atlas;

	// eng: number of files with particle copies
	// rus: количество файлов с копиями частиц
	int k_copy;

	// eng: list of files with particle copies
	// rus: список файлов с копиями частиц
	MP_Copy** m_copy;

	// eng: number of open files
	// rus: количество открытых файлов
	int k_ptc;

	// eng: list of open files
	// rus: список открытых файлов
	MP_PTC** m_ptc;

	// eng: settings to initialize emitters by default
	// rus: настройки для инициализации эмиттеров по умолчанию

	int interpolation;			// eng: method of interpolation applied to loaded emitters
								// rus: способ применения интерполяции к загружаемым эмиттерам

	int position_mode;			// eng: mode of changing emitter position
								// rus: режим изменения позиции эмиттера

	int atlas_width,atlas_height;
	int atlas_frame_step;
	float atlas_scale_step;

	bool copy_mode;				// eng: mode for working with files containing particle copies. false - file is deleted, when there are no emitters, that use it, true - file is deleted at program termination
								// rus: режим работы файлов с копиями частиц с копиями. false - файл удаляется, когда нет эмиттеров, которые его используют, true - файл удаляется при завершении работы программы

	HM_EMITTER next_descriptor;
	int next_index;

	bool is_new_atlas;

public:
	static MP_Device_WRAP* device;

protected:
	MP_Manager();

public:
	static MP_Manager& GetInstance();

	// eng: Cleaning up
	// rus: Очистка
	virtual void Destroy();

	// eng: Initialization
	// rus: Инициализация
	void Initialization(int axis, MP_Platform* platform, int interpolation, int position_mode, int atlas_width=1024, int atlas_height=1024, int atlas_frame_step=1, float atlas_starting_scale=1.f, float atlas_scale_step=0.1f, bool copy_mode=true);

	// eng: Returning the number of emitters
	// rus: Возвращение количества эмиттеров
	int GetEmitterCount(){return k_emitter;}

	// eng: Returning descriptor of first emitter. 
	// rus: Получение дескриптора первого эмиттера
	HM_EMITTER GetFirstEmitter();

	// eng: Returning descriptor of next emitter. 
	// rus: Получение дескриптора следующего эмиттера
	HM_EMITTER GetNextEmitter(HM_EMITTER hmEmitter);

	// eng: Returning the emitter by its descriptor
	// rus: Возвращение эмиттера по дескриптору
	MP_Emitter* GetEmitter(HM_EMITTER hmEmitter);

	// eng: Returning the emitter by name
	// rus: Возвращание эмиттера по имени
	MP_Emitter* GetEmitterByName(const char* name);

	// eng: Loading all emitters from emitters folder
	// rus: Загрузка всех эмиттеров из всех файлов
	void LoadAllEmitters();

	// eng: Loading all the emitters and animated folders from the file specified
	// rus: Загрузка всех эмиттеров из указанного файла. Загружаются эмиттеры и анимированные папки
	HM_FILE LoadEmittersFromFile(const char* file);
	HM_FILE LoadEmittersFromFileInMemory(const char* address);


	// eng: Deleting specified emitter 
	// rus: Удаление указанного эмиттера
	int DeleteEmitter(HM_EMITTER hmEmitter);

	// eng: Closing file
	// rus: Выгрузка одного файла
	int CloseFile(HM_FILE hmFile);

	// eng: Closing all files
	// rus: Выгрузка всех файлов
	void CloseFiles();

	// eng: Duplicating specified emitter
	// rus: Дублирование указанного эмиттера
	HM_EMITTER DuplicateEmitter(HM_EMITTER hmEmitter);

	// eng: Updating emitters taking into account the passed time
	// rus: Обновление эмиттеров по таймеру
	int UpdateByTimer();

	// eng: Updating emitters
	// rus: Обновление эмиттеров
	void Update(double time);

	// eng: Rendering all emitters
	// rus: Рисование эмиттеров
	int Render();

	// eng: Stopping all the emitters
	// rus: Остановка эмиттеров
	void Stop();

	// eng: Refreshing textural atlases
	// rus: Построение текстурного атласа
	void RefreshAtlas();

	// eng: Restoring textural atlas in cases of loosing textures
	// rus: Восстановление текстурного атласа в случае потери текстур
	void RestoreAtlas();

	// eng: Returns the number of textural atlases
	// rus: Возвращает количество текстурных атласов
	int GetAtlasCount(){return k_atlas;}

	// eng: Returns textural atlas
	// rus: Возвращает текстурный атлас
	MP_Atlas* GetAtlas(int index){return m_atlas[index];}

	// eng: Adding new emitter into array
	// rus: Добавление нового эмиттера в массив
	void AddEmitter(MP_Emitter* emitter);

	// eng: Loading emitter
	// rus: Загрузка конкретного эмиттера
	MP_Emitter* LoadEmitter(HM_FILE file, const char* path);

	// eng: Loading folder
	// rus: Загрузка папки
	void LoadFolder(HM_FILE file, const char* path);

	// eng: Adding file with particles copy
	// rus: Добавление файла с копией частиц
	MP_Copy* AddCopy(MP_Emitter* emitter);

	// eng: Deleting file with particles copy
	// rus: Удаление файла с копией частиц
	void DeleteCopy(MP_Copy* copy);

	// eng: Getting mode of working with files containing particle copies
	// rus: Получение режима работы файлов копий частиц
	bool GetCopyMode(){return copy_mode;}

	// eng: Searching among files containing particle copies by specified emitter id
	// rus: Поиск среди файлов копий частиц соответствующего указанному идентификатору эмиттера
	MP_Copy* FindCopy(unsigned int emitter_id);

	// eng: Adding open file
	// rus: Добавление открытого файла
	void AddPTC(MP_PTC* ptc_file);

	// eng: Deleting open file
	// rus: Удаление открытого файла
	int DeletePTC(HM_FILE hmFile);

	// eng: Returns time in milliseconds
	// rus: Возвращает время в милисекундах
	unsigned long GetTick(){return platform->GetTick();}
	
	// eng: Returns path to folder with emitters
	// rus: Возвращает путь к папке с ptc-файлами
	const char* GetPathToPTC(){return platform->GetPathToPTC();}

	// eng: Returns path to folder with textures
	// rus: Возвращает путь к папке с текстурами
	const char* GetPathToTexture(){return platform->GetPathToTexture();}

	// eng: Returns path to folder which could be used by wrapper to store temporary files
	// rus: Возвращает путь к временной папке
	const char* GetPathToTemp(){return platform->GetPathToTemp();}

	// eng: Finds the first ptc-file in emitters folder
	// rus: Поиск первого ptc-файла в папке с эмиттерами
	const char* GetFirstFile(){return platform->GetFirstFile();}

	// eng: Finds the next ptc-file in emitters folder
	// rus: Поиск очередного ptc-файла в папке с эмиттерами
	const char* GetNextFile(){return platform->GetNextFile();}

	// eng: Converts wchar_t string into utf8
	// rus: Конвертирует строку типа wchar_t в строку типа utf8
	const char* wchar_to_utf8(const wchar_t* str){return platform->wchar_to_utf8(str);}

	// eng: Converts utf8 string into wchar_t
	// rus: Конвертирует строку типа utf8 в строку типа wchar_t
	const wchar_t* utf8_to_wchar(const char* str){return platform->utf8_to_wchar(str);}

	// eng: Opens ptc-file
	// rus: Открывает ptc-файл
	HM_FILE OpenPTC(const char* ptc_file){return platform->OpenPTC(ptc_file);}

	// Deleting of file (analogue remove())
	// Удаление файла (аналог remove())
	int RemoveFile(const char* file){return platform->RemoveFile(file);}
};

struct MP_POSITION
{
	float x,y,z;

	MP_POSITION();
	MP_POSITION(float x,float y,float z);
	MP_POSITION(float x,float y);

	MP_POSITION operator+(const MP_POSITION& pos);
	MP_POSITION operator+=(const MP_POSITION& pos);
	MP_POSITION operator-(const MP_POSITION& pos);
	MP_POSITION operator-=(const MP_POSITION& pos);

	float Length();
	
	void Normalize();
	
	// rus: Векторное произведение
	void Cross(const MP_POSITION* pV1, const MP_POSITION* pV2);
	// rus: Скалярное произведение
	float Dot(const MP_POSITION* p);
};


// ---------------------------------------------------------------------------------------

// eng: Class, specialized for work with the emitters
// rus: Класс, который хранит загруженные эмиттеры

#define MAGIC_STATE_STOP		0	// eng: emitter is not working 
									// rus: эмиттер не работает

#define MAGIC_STATE_UPDATE		1	// eng: emitter is updated and rendered 
									// rus: эмиттер обновляется и рисуется

#define MAGIC_STATE_INTERRUPT	2	// eng: emitter interrupts, i.e. is working while there are "alive" particles 
									// rus: эмиттер обновляется и рисуется до момента уничтожения всех созданных частиц, новые частицы больше не создаются

#define MAGIC_STATE_VISIBLE		3	// eng: Emitter is only rendered 
									// rus: эмиттер только рисуется

class MP_Emitter
{
protected:
	int state;					// eng: state
								// rus: статус
	
	MP_Manager* owner;			// eng: pointer to manager
								// eng: указатель на менеджер

	HM_EMITTER emitter;			// eng: descriptor of emitter
								// rus:дескриптор эмиттера

	float z;					// eng: coordinate z of emitter
								// ru: координата z эмиттера

	bool first_restart;			// eng: indicates that emitter does not work
								// rus: признак того, что эмиттер еще не выставлялся на стартовую позицию анимации

public:
	MP_Copy* copy;				// rus: file with particles copy
								// rus: файл с копией частиц

public:
	bool is_atlas;				// eng: indicates that atlas for this emitter was created
								// rus: признак того, что атлас для данного эмиттера уже был построен

	std::string restore_file;	// rus: file for restoring of emitter textures
								// rus: файл для восстановления текстур эмиттера

	static MP_Device_WRAP* device;
	
public:
	MP_Emitter(HM_EMITTER emitter, MP_Manager* owner);
	virtual ~MP_Emitter();

	// eng: Returns manager
	// eng: Возвращает менеджер
	MP_Manager* GetManager(){return owner;}

	MP_Emitter& operator=(const MP_Emitter& from);

	// eng: Returning the descriptor of the emitter to work with API
	// Возвращение дескриптора эмиттера, чтобы работать с API
	HM_EMITTER GetEmitter(){return emitter;}

	// eng: Returning the name of the emitter
	// rus: Возвращение имени эмиттера
	const char* GetEmitterName();

	// eng: Restarting of emitter
	// rus: Установка эмиттера на стартовую позицию
	void Restart();

	// eng: Position of emitter
	// rus: Позиция эмиттера
	void GetPosition(MP_POSITION& position);
	void SetPosition(MP_POSITION& position);

	// eng: Moving the emitter to the position specified allowing restart. 
	// rus: Перемещение эмиттера в указанную позицию с возможностью перезапуска. При перемещении все существующие частицы перемещаются вместе с эмиттером
	void Move(MP_POSITION& position, bool restart=false);

	// eng: Offsetting the current emitter position by the value specified
	// rus: Смещение текущей позиции эмиттера на указанную величину. Будет использован текущий режим перемещения эмиттера
	void Offset(MP_POSITION& offset);

	// eng: Direction of emitter
	// rus: Направление эмиттера
	void SetDirection(MAGIC_DIRECTION* direction);
	void GetDirection(MAGIC_DIRECTION* direction);

	// eng: Setting the emitter direction to the specified value with the restart ability
	// rus: Поворот эмиттера в указанное направление с возможностью перезапуска. При повороте все существующие частицы поворачиваются вместе с эмиттером
	void Direct(MAGIC_DIRECTION* direction, bool restart=false);
	
	// eng: Rotating of the emitter by the specified value
	// rus: Вращение эмиттера на указанную величину. Будет использован текущий режим вращения эмиттера
	void Rotate(MAGIC_DIRECTION* offset);

	// eng: Returning the scale of the emitter
	// rus: Возвращение масштаба эмиттера
	float GetScale();

	// eng: Setting the scale of the emitter
	// rus: Установка масштаба эмиттера
	void SetScale(float scale);

	// eng: Returning the state of the emitter
	// rus: Возвращение статуса эмиттера
	int GetState(){return state;}

	// eng: Setting the state of the emitter
	// rus: Установка статуса эмиттера
	void SetState(int state);

	// eng: Updating emitter
	// rus: Обновление эмиттера
	void Update(double time);

	// eng: Emitter visualization
	// rus: Отрисовка эмиттера. Возвращается количество нарисованных частиц
	int Render();
};

// eng: Class storing the files with particles copies
// rus: Класс, который хранит файлы с копиями частиц
class MP_Copy
{
protected:
	int reference;
	std::string file;
	HM_STREAM ram;

	unsigned int emitter_id;

	static int file_count;
	static std::string file_name;

public:
	static MP_Manager* MP;

public:
	MP_Copy(MP_Emitter* emitter);
	virtual ~MP_Copy();

	// eng: Cleaning up
	// rus: Очистка
	void Clear();

	// eng: Increasing of reference count
	// rus: Увеличение числа ссылок на файл
	void IncReference(MP_Emitter* emitter);
	// eng: Decreasing of reference count
	// rus: Уменьшение числа ссылок на файл
	void DecReference();

	// eng: Loading of particles from file to emitter
	// rus: Загрузка частиц из файла в эмиттер
	void LoadParticles(MP_Emitter* emitter);

	// rus: Returns of emitter ID, for which file was created
	// rus: Возвращает идентификатор эмиттера, для которого создан файл
	unsigned int GetEmitterID(){return emitter_id;}
};

// eng: Class storing the textural atlas. This class will be abstract
// rus: Класс, который хранит текстурный атлас. Этот класс будет абстрактным
class MP_Atlas
{
protected:
	int atlas_width, atlas_height;
	std::string file_name;

public:
	static MP_Manager* MP;
	static MP_Device_WRAP* device;

public:
	MP_Atlas(int width, int height, const char* file);
	virtual ~MP_Atlas(){}

	// eng: Destroy atlas texture
	// rus: Уничтожить текстуру атласа
	virtual void Destroy()=0;

	// eng: Returns sizes
	// rus: Возвращает размеры
	void GetSize(int& atlas_width,int& atlas_height){atlas_width=this->atlas_width;atlas_height=this->atlas_height;}

	// eng: Returns file name of texture
	// rus: Возвращает имя файла текстуры
	const char* GetFile(){return file_name.c_str();}

	// eng: Loading of frame texture
	// rus: Загрузка текстурного кадра
	virtual void LoadTexture(const MAGIC_CHANGE_ATLAS* c)=0;

	// eng: Cleaning up of rectangle
	// rus: Очистка прямоугольника
	virtual void CleanRectangle(const MAGIC_CHANGE_ATLAS*){}

	// eng: Beginning of emitter drawing
	// rus: Начало отрисовки эмиттера
	virtual void BeginDrawEmitter(MP_Emitter*){}

	// eng: End of emitter drawing
	// rus: Конец отрисовки эмиттера
	virtual void EndDrawEmitter(MP_Emitter*){}

	// eng: Beginning of particles drawing
	// rus: Начало отрисовки частиц
	virtual void BeginDrawParticles(int particles_count){}

	// eng: End of particles drawing
	// rus: Конец отрисовки частиц
	virtual void EndDrawParticles(){}

	// eng: Particle drawing
	// rus: Отрисовка частицы
	virtual void Draw(MAGIC_PARTICLE_VERTEXES* vertex_rectangle)=0;

	// eng: Setting of intense
	// rus: Установка интенсивности
	virtual void SetIntense(bool intense)=0;
};

struct MP_PTC
{
	HM_FILE hmFile;
	HM_STREAM hmStream;
	const char* data;

	MP_PTC();
	virtual ~MP_PTC();
	int Close();
};

struct MP_VIEWPORT
{
	unsigned int x,y;
	unsigned int width, height;
	float min_z,max_z;
};

// eng: Class controlling drawing. This class will be abstract
// rus: Класс, который управляет рисованием. Этот класс будет абстрактным
class MP_Device
{
protected:
	int window_width;
	int window_height;

public:
	static MP_Manager* MP;

public:
	MP_Device(int width, int height);
	virtual ~MP_Device();

	// eng: Returns window size
	// rus: Возвращает размер окна
	void GetWindowSize(int& wi, int& he){wi=window_width; he=window_height;}

	// eng: Creating
	// rus: Создание
	virtual bool Create()=0;

	// eng: Destroying
	// rus: Уничтожение
	virtual void Destroy();

	// eng: Beginning of scene drawing
	// rus: Начало отрисовки сцены
	virtual void BeginScene();

	// eng: End of scene drawing
	// rus: Конец отрисовки сцены
	virtual void EndScene()=0;

	// eng: Indicates that device is lost
	// rus: Проверка на потерю устройства рисования
	virtual bool IsLost()=0;

	// eng: Setting of scene
	// rus: Настройка сцены
#ifdef MAGIC_3D
	virtual void SetScene3d()=0;
#else
	virtual void SetScene3d(){}
#endif

	// eng: Creating of atlas object
	// rus: Создание объекта атласа
	virtual MP_Atlas* NewAtlas(int width, int height, const char* file);
};

// ---------------------------------------------------------------------------------------

// eng: Saves TGA for testing
// rus: Сохраняет TGA для тестирования
extern void SaveTGA(int w, int h, unsigned char* pixel, const char* file);

#endif