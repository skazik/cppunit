#include <iostream>
#include <string>
#include <list>
#include <cppunit/TestCase.h>
#include <cppunit/TestFixture.h>
#include <cppunit/ui/text/TextTestRunner.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/TestRunner.h>
#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/XmlOutputter.h>
#include <cppunit/XmlOutputterHook.h>
#include <cppunit/tools/XmlElement.h>
#include <cppunit/tools/XmlDocument.h>
#include <cppunit/tools/StringTools.h>
#include <netinet/in.h>
#include <unistd.h>

#include <cppunit/TestListener.h>
#include <cppunit/Test.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <memory.h>
#include <sys/stat.h>

using namespace CppUnit;
using namespace std;

// set TEST_CPPUNIT_DURATION 0 for real testing
#define TEST_CPPUNIT_DURATION 1
#define MODULE_PREFIX  "|%lld| CPPUNIT | "
#include "utils.h"
#include "cppunit.h"
static TestListDesc_t gsTestList[CPPUNIT_TEST_COUNT_MAX];

static char gsTestResultNamePrefix[32] = "";
#define TEST_RESULT_PATH                    "./test-reports/"
#ifdef JUINT_XML_RESULT_ENABLED
#define TEST_RESULT_JUNIT   TEST_RESULT_PATH "j%s_%d.xml"
#define TEST_RESULT_CPPUNIT                  "c%s_%d.xml"
#else
#define TEST_RESULT_CPPUNIT TEST_RESULT_PATH "c%s_%d.xml"
#endif

// static definitions and CPPUNIT re-definitions for SOIT FrameWork
static char gsActualTestName[0xff];
#define SAMPLE_CPPUNIT_ASSERT_EQUAL_MESSAGE(sourceline, message, expected, actual)     \
  ( CPPUNIT_NS::assertEquals( (expected),   \
                              (actual),     \
                              (sourceline), \
                              (message) ) )

#define CASE_SAMPLE_CPPUNIT_TEST(__test_id_name , testMethod )               \
    case __test_id_name: {                                 \
    sprintf(gsActualTestName, "%s_" #__test_id_name, gsTestResultNamePrefix); \
    CPPUNIT_TEST_SUITE_ADD_TEST(                           \
        ( new CPPUNIT_NS::TestCaller<TestFixtureType>(    \
                  context.getTestNameFor(gsActualTestName),   \
                  &TestFixtureType::testMethod,           \
                  context.makeFixture() ) ) ) ; } break;

#define CASE_SAMPLE_CPPUNIT_TEST_IDX(__test_id_name , testMethod , __idx)               \
    case __test_id_name: {                                 \
    sprintf(gsActualTestName, "%s_" #__test_id_name "%d", gsTestResultNamePrefix, __idx); \
    CPPUNIT_TEST_SUITE_ADD_TEST(                           \
        ( new CPPUNIT_NS::TestCaller<TestFixtureType>(    \
                  context.getTestNameFor(gsActualTestName),   \
                  &TestFixtureType::testMethod,           \
                  context.makeFixture() ) ) ) ; } break;


#define MSG_BUFFER_SIZE_MAX 0x400
static char gsErrorMessage[MSG_BUFFER_SIZE_MAX];
static char gsSourceline[MSG_BUFFER_SIZE_MAX];

///////////////////////
/// \brief The TimingListener class
///
class TimingListener : public TestListener
{
public:
    TimingListener ()
    {
        _chron_initialized = false;
    }

    TimingListener (long long timetoinit)
    {
        _chron_initialized = false;
        if (timetoinit > 0) {
            _chron_initialized = true;
            _chronometer = timetoinit;
        }
    }

    void startTest( CppUnit::Test *test )
    {
        if (!_chron_initialized)
            _chronometer = UTILS_current_timestamp();
        return;
        // suppress
        test++;
    }

    void endTest( CppUnit::Test *test )
    {
        _chronometer = UTILS_current_timestamp() - _chronometer;
        std::string testName = test->getName();
        timing_map.insert(std::pair<string, long long>(testName, _chronometer));
    }

    // private interface
    std::map <string, long long> getTimes()
    {
        return timing_map;
    }

private:
    bool _chron_initialized;
    long long _chronometer;
    std::map <string, long long> timing_map;
};

///////////////////////////
/// \brief The MyXmlOutputterHook class
///
class MyXmlOutputterHook : public XmlOutputterHook
 {
private:
    char* getTimeStamp()
    {
        time_t t = time(NULL);
        struct tm tm = *localtime(&t);
        sprintf(timestr, "%d-%02d-%02dT%02d:%02d:%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
        return timestr;
    }

public:
   MyXmlOutputterHook(std::map <string, long long> timing_map)
   {
       this->timing = timing_map;
   }

   virtual ~MyXmlOutputterHook()
   {
   }

   virtual void successfulTestAdded( XmlDocument *document,
                                     XmlElement *testElement,
                                     Test *test )
   {
       // add timing
       // note: Bamboo expect it in seconds while we count in ms
       std::map<std::basic_string<char>, long long int>::iterator it = timing.find(test->getName());
       long long elapsed = (it->second) / 1000;
       timing.erase(it);
       testElement->addElement( new CppUnit::XmlElement("Time", elapsed));

       return;
       // dead code: to suppress compiler warnings unused parameter
       char* p = (char*) document; p++;
   }

   virtual void failTestAdded( XmlDocument *document,
                               XmlElement *testElement,
                               Test *test,
                               TestFailure *failure )
   {
       // add timing
       // note: Bamboo expect it in seconds while we count in ms
       std::map<std::basic_string<char>, long long int>::iterator it = timing.find(test->getName());
       long long elapsed = (it->second) / 1000;
       timing.erase(it);
       testElement->addElement( new CppUnit::XmlElement("Time", elapsed));

       return;
       // dead code: to suppress compiler warnings unused parameter
       char* p = (char*) document; p = (char*) failure; p++;
   }

   virtual void statisticsAdded( XmlDocument *document,
                                 XmlElement *statisticsElement )
   {
       statisticsElement->addElement( new CppUnit::XmlElement("Time", 0));
       statisticsElement->addElement( new CppUnit::XmlElement("Timestamp", getTimeStamp()));

       return;
       // dead code: to suppress compiler warnings unused parameter
       char* p = (char*) document; p++;
   }

private:
    std::map <string, long long> timing;
    char timestr[64];
};

////////////////
/// \brief The CBasicMath class for self test only
///
class CBasicMath
{
public:
   int Addition(int x, int y);
   int Multiply(int x, int y);
   int FailureTest();
};

int CBasicMath::Addition(int x, int y)
{
    // sleep for self test unitest duration eval
    usleep(1000000);
    return (x + y);
}

int CBasicMath::Multiply(int x, int y)
{
    // sleep for self test unitest duration eval
    usleep(2000000);
    return (x * y);
}

int CBasicMath::FailureTest()
{
    // sleep for self test unitest duration eval
    usleep(3000000);
    return -1;
}

/////////////////////////////////////////////////
/// \brief The CppUnitSoitFW class - main Soit Framework unitest class
///
class CppUnitSoitFW : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CppUnitSoitFW);
    for (int idx = 0; idx < CPPUNIT_TEST_COUNT_MAX; idx++)
    {   // "switch is used due to macro with explicit name of the function should be set
        switch (gsTestList[idx].test_index)
        {
        // Soit unitest section
        CASE_SAMPLE_CPPUNIT_TEST(eScriptValidation, CompareIntValues);
        CASE_SAMPLE_CPPUNIT_TEST(eFinalReturnValue, CompareIntValues);
        CASE_SAMPLE_CPPUNIT_TEST(eExecute, CompareIntValues);
        CASE_SAMPLE_CPPUNIT_TEST_IDX(eParseResult, CompareListOfIntValues, idx);

        // self test section
        case 251: CPPUNIT_TEST(selfTestAdd); break;
        case 252: CPPUNIT_TEST(selfTestMultiply); break;
        case 253: CPPUNIT_TEST(selfTestFailure); break;

        // end of the list handling and default
        case CPPUNIT_TEST_END_OF_LIST:
            idx = CPPUNIT_TEST_COUNT_MAX;
            break;
        default:
            break; // wrong test id - just skip for now
        }
    }
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp(void);
    void tearDown(void);

protected:
    void selfTestAdd(void);
    void selfTestMultiply(void);
    void selfTestFailure(void);

protected: // actual methods for mSoitTestObj
    void CompareIntValues(void);
    void CompareListOfIntValues(void);

private:
    CBasicMath *mTestObj;

public:
    static unsigned int mValIdx;
    static unsigned int mValCount;
    static unsigned int mValue1[CPPUNIT_TEST_RESULTS_MAX];
    static unsigned int mValue2[CPPUNIT_TEST_RESULTS_MAX];
    static int mSourceLine;
    static char *mMessage;
    static char *mSourceFile;
};

// init class static variables
unsigned int CppUnitSoitFW::mValIdx = 0;
unsigned int CppUnitSoitFW::mValCount = 0;
unsigned int CppUnitSoitFW::mValue1[CPPUNIT_TEST_RESULTS_MAX];
unsigned int CppUnitSoitFW::mValue2[CPPUNIT_TEST_RESULTS_MAX];
char *CppUnitSoitFW::mMessage = gsErrorMessage;
char *CppUnitSoitFW::mSourceFile = gsSourceline;
int CppUnitSoitFW::mSourceLine = 0;

//-----------------------------------------------------------------------------

void CppUnitSoitFW::selfTestAdd(void)
{
    CPPUNIT_ASSERT_EQUAL(5, mTestObj->Addition(2,3));
}

void CppUnitSoitFW::selfTestMultiply(void)
{
    CPPUNIT_ASSERT_EQUAL(6, mTestObj->Multiply(2,3));
}

void CppUnitSoitFW::selfTestFailure(void)
{
    CPPUNIT_ASSERT_EQUAL(0, mTestObj->FailureTest());
}

void CppUnitSoitFW::CompareIntValues(void)
{
    SAMPLE_CPPUNIT_ASSERT_EQUAL_MESSAGE(CPPUNIT_NS::SourceLine( CppUnitSoitFW::mSourceFile, CppUnitSoitFW::mSourceLine ),
                                       CppUnitSoitFW::mMessage,  CppUnitSoitFW::mValue1[0], CppUnitSoitFW::mValue2[0]);
}

void CppUnitSoitFW::CompareListOfIntValues(void)
{
    if (CppUnitSoitFW::mValIdx < CppUnitSoitFW::mValCount)
    {
        int idx = CppUnitSoitFW::mValIdx++;
        SAMPLE_CPPUNIT_ASSERT_EQUAL_MESSAGE(CPPUNIT_NS::SourceLine( CppUnitSoitFW::mSourceFile, CppUnitSoitFW::mSourceLine ), CppUnitSoitFW::mMessage,
                                           CppUnitSoitFW::mValue1[idx], CppUnitSoitFW::mValue2[idx]);
    }
}

void CppUnitSoitFW::setUp(void)
{
    mTestObj = new CBasicMath();
}

void CppUnitSoitFW::tearDown(void)
{
    delete mTestObj;
}

//-----------------------------------------------------------------------------

CPPUNIT_TEST_SUITE_REGISTRATION( CppUnitSoitFW );

int exec_cppunittest(int testid, int count, TestListDesc_t *testList, long long inittime)
{
    if (!(count > 0 && testList))
        return EXEC_CPPUNITTEST_INVALID_ARG;
    memcpy(gsTestList, testList, sizeof(TestListDesc_t)*MIN(count, CPPUNIT_TEST_COUNT_MAX));

    // informs test-listener about testresults
    CPPUNIT_NS::TestResult testresult;

    // register listener for collecting the test-results
    CPPUNIT_NS::TestResultCollector collectedresults;
    testresult.addListener (&collectedresults);

    // register listener for per-test progress output
    // CPPUNIT_NS::TestListener progress;
    TimingListener progress(inittime);
    testresult.addListener(&progress);

    // insert test-suite at test-runner by registry
    CPPUNIT_NS::TestRunner testrunner;
    testrunner.addTest (CPPUNIT_NS::TestFactoryRegistry::getRegistry().makeTest ());
    testrunner.run(testresult);

    // output results in compiler-format
    CPPUNIT_NS::CompilerOutputter compileroutputter(&collectedresults, std::cerr);
    compileroutputter.write ();

    // Output XML for CPPunit plugin
    // check if output directory exists or create
    struct stat dirStat;
    if (!((stat(TEST_RESULT_PATH, &dirStat) == 0) && (((dirStat.st_mode) & S_IFMT) == S_IFDIR))) {
        // create it
        mkdir(TEST_RESULT_PATH, 0776);
    }

    char cresname[64];
    sprintf(cresname, TEST_RESULT_CPPUNIT, gsTestResultNamePrefix, testid);
    ofstream xmlFileOut(cresname);
    XmlOutputter xmlOut(&collectedresults, xmlFileOut);
    XmlOutputterHook *hook = (XmlOutputterHook*) new MyXmlOutputterHook(progress.getTimes());
    xmlOut.addHook(hook);
    xmlOut.write();

    int child_pid = 0;
#ifdef JUINT_XML_RESULT_ENABLED
    child_pid = fork();
    if (0 == child_pid)
    {   // convert to JUnit format
        char jresname[64];
        sprintf(jresname, TEST_RESULT_JUNIT, gsTestResultNamePrefix, testid);
        //$ xsltproc -o junitResults.xml cpp2junit.xslt cppunitResults.xml
        const char *argv[] = {"/usr/bin/xsltproc", "-o", jresname, "cpp2junit.xslt", cresname, NULL};
        execvp(argv[0], (char* const*) argv);
        _exit(0);
    }
#endif
    return child_pid;
}

///////////////////////////////
/// \brief child_wait
/// \param child_pid
///
void child_wait (int child_pid)
{   // wait child exit if vbalid
    if (child_pid > 0) {
        int child_status;
        pid_t tpid = wait(&child_status);
        while(tpid != child_pid)
          tpid = wait(&child_status);
    }
}

//////////////////////
/// \brief cppunit_selftest
/// \return 0 success or error
///
void cppunit_selftest()
{
    // TestListDesc_t list1[] = {251,253,CPPUNIT_TEST_END_OF_LIST};
    TestListDesc_t list2[] = {{251},{252},{253},{CPPUNIT_TEST_END_OF_LIST}};
    // child_wait(exec_cppunittest(111, sizeof(list1)/sizeof(TestListDesc_t), list1, 0));
    child_wait(exec_cppunittest(222, sizeof(list2)/sizeof(TestListDesc_t), list2, 0));
}

////////////////////////////
/// \brief cppunit_compare_values
/// \param file
/// \param line
/// \param expected
/// \param retval
/// \param test_id
/// \param initime
/// \param msg
///
void cppunit_compare_values(const char *file, int line, int expected, int retval, eTestName_t test_id, long long initime, char *msg)
{
    if (test_id < CPPUNIT_TEST_END_OF_LIST) {
        CppUnitSoitFW::mValue1[0] = expected;
        CppUnitSoitFW::mValue2[0] = retval;
        snprintf(CppUnitSoitFW::mMessage, MSG_BUFFER_SIZE_MAX, "%s", (msg == NULL) ? gbUnitestErrorMessage : msg);
        snprintf(CppUnitSoitFW::mSourceFile, MSG_BUFFER_SIZE_MAX, "%s", file);
        CppUnitSoitFW::mSourceLine = line;

        TestListDesc_t list1[] = {{test_id}, {CPPUNIT_TEST_END_OF_LIST}};
        child_wait(exec_cppunittest((int) test_id, sizeof(list1)/sizeof(TestListDesc_t), list1, initime));
    }
}

////////////////////////////
/// \brief cppunit_compare_listvals
/// \param file
/// \param line
/// \param count
/// \param vals
/// \param test_id
/// \param initime
/// \param msg
///
void cppunit_compare_listvals(const char *file, int line, int count, unsigned int vals[], eTestName_t test_id, long long initime, char *msg)
{
    // reserved for 100 eParseResult MAX only, range [3..102]
    if (test_id == eParseResult) {
        CppUnitSoitFW::mValIdx = 0;
        CppUnitSoitFW::mValCount = count;
        snprintf(CppUnitSoitFW::mMessage, MSG_BUFFER_SIZE_MAX, "%s", (msg == NULL) ? gbUnitestErrorMessage : msg);
        snprintf(CppUnitSoitFW::mSourceFile, MSG_BUFFER_SIZE_MAX, "%s", file);
        CppUnitSoitFW::mSourceLine = line;

        TestListDesc_t list1[CPPUNIT_TEST_COUNT_MAX];
        int i = 0;
        for (; i < count; i++)
        {
            CppUnitSoitFW::mValue1[i] = vals[i*2];
            CppUnitSoitFW::mValue2[i] = vals[i*2+1];
            list1[i].test_index = test_id;
        }
        list1[i].test_index = CPPUNIT_TEST_END_OF_LIST;
        child_wait(exec_cppunittest((int) test_id, sizeof(list1)/sizeof(TestListDesc_t), list1, initime));
    }
}

//////////////////////////////
/// \brief cppunit_setprefix
/// \param prefix
///
void cppunit_setprefix(const char *prefix)
{
    // cut to the real name - remove path
    char *pStart = (char*) strrchr(prefix, '/');
    if (pStart == NULL)
        pStart = (char*) prefix;
    else
        pStart++;
    strncpy(gsTestResultNamePrefix, pStart, sizeof(gsTestResultNamePrefix));
    gsTestResultNamePrefix[sizeof(gsTestResultNamePrefix)-1]='\0';

    // cut file extension if any
    pStart = strrchr(gsTestResultNamePrefix, '.');
    if (pStart)
        *pStart = '\0';

}
