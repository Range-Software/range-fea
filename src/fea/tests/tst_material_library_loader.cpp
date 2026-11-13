#include <QtTest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSignalSpy>
#include <QSemaphore>
#include <thread>
#include <QTemporaryDir>
#include <QTimer>
#include <atomic>

#include "material_library_loader.h"

class MaterialLibraryLoaderTest : public QObject
{
    Q_OBJECT

    static QString writeMaterial(const QString &directory, const RMaterial &material)
    {
        const QString path = directory + "/" + material.getID().toString(QUuid::WithoutBraces) + ".json";
        material.write(path);
        return path;
    }

    private slots:
        void loadsOffGuiThread()
        {
            QTemporaryDir directory;
            QVERIFY(directory.isValid());
            RMaterial material = RMaterial::generateDefault();
            const QString path = writeMaterial(directory.path(),material);
            MaterialLibraryLoader loader(directory.path());
            std::atomic<bool> workerSignal{false};
            connect(&loader,&MaterialLibraryLoader::progressChanged,&loader,[&](){
                workerSignal = QThread::currentThread() != QCoreApplication::instance()->thread();
            },Qt::DirectConnection);
            bool guiEventDelivered = false;
            QTimer::singleShot(0,this,[&](){ guiEventDelivered = true; });
            QSignalSpy finished(&loader,&QThread::finished);
            loader.start();
            QTRY_COMPARE(finished.count(),1);
            QVERIFY(guiEventDelivered);
            QVERIFY(workerSignal.load());
            QVERIFY(loader.errors.isEmpty());
            QCOMPARE(loader.materials.size(),1);
            QCOMPARE(loader.materials.value(path).getID(),material.getID());
            QVERIFY(loader.materials.value(path).propertiesEqual(material));
        }

        void continuesAfterBadFile()
        {
            QTemporaryDir directory;
            QVERIFY(directory.isValid());
            QFile bad(directory.filePath("000-bad.json"));
            QVERIFY(bad.open(QIODevice::WriteOnly));
            bad.write("{ invalid json");
            bad.close();
            RMaterial material = RMaterial::generateDefault();
            const QString good = writeMaterial(directory.path(),material);
            MaterialLibraryLoader loader(directory.path());
            loader.start();
            QVERIFY(loader.wait(5000));
            QCOMPARE(loader.errors.size(),1);
            QVERIFY(loader.errors.first().contains("000-bad.json"));
            QCOMPARE(loader.materials.size(),1);
            QVERIFY(loader.materials.contains(good));
            QVERIFY(QFile::exists(bad.fileName()));
        }

        void convertsLegacyFiles_data()
        {
            QTest::addColumn<QString>("extension");
            QTest::newRow("ASCII") << "ramt";
            QTest::newRow("binary") << "rbmt";
        }

        void convertsLegacyFiles()
        {
            QFETCH(QString,extension);
            QTemporaryDir directory;
            QVERIFY(directory.isValid());
            RMaterial material = RMaterial::generateDefault();
            const QString legacy = directory.filePath("legacy." + extension);
            material.write(legacy);
            MaterialLibraryLoader loader(directory.path());
            loader.start();
            QVERIFY(loader.wait(5000));
            QVERIFY2(loader.errors.isEmpty(),qPrintable(loader.errors.join('\n')));
            QCOMPARE(loader.materials.size(),1);
            QVERIFY(!QFile::exists(legacy));
            const QString canonical = directory.filePath(material.getID().toString(QUuid::WithoutBraces) + ".json");
            QVERIFY(loader.materials.contains(canonical));
            RMaterial converted;
            converted.read(canonical);
            QCOMPARE(converted.getID(),material.getID());
            QVERIFY(converted.propertiesEqual(material));
        }

        void keepsNewerCanonicalFile()
        {
            QTemporaryDir directory;
            QVERIFY(directory.isValid());
            RMaterial legacy = RMaterial::generateDefault();
            legacy.setName("Old name");
            const QString legacyPath = directory.filePath("000-legacy.ramt");
            legacy.write(legacyPath);
            RMaterial current(legacy);
            current.setName("Current name");
            const QString canonical = writeMaterial(directory.path(),current);
            MaterialLibraryLoader loader(directory.path());
            loader.start();
            QVERIFY(loader.wait(5000));
            QVERIFY(loader.errors.isEmpty());
            QCOMPARE(loader.materials.size(),1);
            QCOMPARE(loader.materials.value(canonical).getName(),QString("Current name"));
            QVERIFY(!QFile::exists(legacyPath));
        }

        void destructionJoinsActiveWorker()
        {
            QTemporaryDir directory;
            QVERIFY(directory.isValid());
            for (int i=0; i<3; ++i)
            {
                writeMaterial(directory.path(),RMaterial::generateDefault());
            }
            QSemaphore entered;
            QSemaphore resume;
            std::atomic<int> completed{0};
            auto *loader = new MaterialLibraryLoader(directory.path());
            connect(loader,&MaterialLibraryLoader::progressChanged,loader,[&](int count,int){
                completed = count;
                if (count == 1)
                {
                    entered.release();
                    resume.acquire();
                }
            },Qt::DirectConnection);
            loader->start();
            if (!entered.tryAcquire(1,5000))
            {
                resume.release();
                delete loader;
                QFAIL("Worker did not load its first file");
            }
            // Simulate an in-flight filesystem operation completing only after
            // the widget starts shutting down. Destruction must join safely and
            // prevent a subsequent file from being loaded.
            std::thread releaseRead([&](){
                while (!loader->isInterruptionRequested())
                {
                    QThread::msleep(1);
                }
                resume.release();
            });
            delete loader;
            releaseRead.join();
            QCOMPARE(completed.load(),1);
        }

        void interruptionStopsBeforeNextFile()
        {
            QTemporaryDir directory;
            QVERIFY(directory.isValid());
            for (int i=0; i<3; ++i)
            {
                writeMaterial(directory.path(),RMaterial::generateDefault());
            }
            MaterialLibraryLoader loader(directory.path());
            connect(&loader,&MaterialLibraryLoader::progressChanged,&loader,[&](int completed,int){
                if (completed == 1)
                {
                    loader.requestInterruption();
                }
            },Qt::DirectConnection);
            loader.start();
            QVERIFY(loader.wait(5000));
            QCOMPARE(loader.materials.size(),1);
            QVERIFY(loader.errors.isEmpty());
        }
};

QTEST_GUILESS_MAIN(MaterialLibraryLoaderTest)
#include "tst_material_library_loader.moc"
