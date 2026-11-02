#include "../src/domain/mapmodel.h"

#include <QJsonArray>
#include <QtTest>

#include <stdexcept>

class MapModelTests : public QObject {
    Q_OBJECT

private slots:
    void startsWithEmptySignalMap();
    void addsAndMovesAccessPoints();
    void validatesAccessPoints();
    void barriersAttenuateSignal();
    void resizePreservesElements();
    void jsonRoundTripPreservesMap();
    void invalidJsonDoesNotChangeMap();
};

void MapModelTests::startsWithEmptySignalMap()
{
    const MapModel map(8, 6);
    QCOMPARE(map.parcelAt(0, 0).signalDbm, -100.0);
    QCOMPARE(map.parcelAt(0, 0).color, MapModel::colorForSignal(-100.0));
}

void MapModelTests::addsAndMovesAccessPoints()
{
    MapModel map(12, 12);
    const int id = map.addAccessPoint(2, 3, 2.4, 18);
    QCOMPARE(map.accessPoints().size(), 1);
    QVERIFY(map.parcelAt(2, 3).signalDbm > -40.0);
    map.moveAccessPoint(id, 8, 9);
    QCOMPARE(map.accessPoints().first().x, 8);
    QCOMPARE(map.accessPoints().first().y, 9);
    QVERIFY(map.parcelAt(2, 3).signalDbm < map.parcelAt(8, 9).signalDbm);
    QVERIFY(map.parcelAt(8, 9).signalDbm > -40.0);
    map.removeAccessPoint(id);
    QCOMPARE(map.accessPoints().size(), 0);
}

void MapModelTests::validatesAccessPoints()
{
    MapModel map(10, 10);
    map.addAccessPoint(1, 1, 5.0, 11);
    QVERIFY_EXCEPTION_THROWN(map.addAccessPoint(1, 1, 2.4, 11), std::invalid_argument);
    QVERIFY_EXCEPTION_THROWN(map.addAccessPoint(10, 0, 2.4, 11), std::invalid_argument);
    QVERIFY_EXCEPTION_THROWN(map.addAccessPoint(2, 2, 6.0, 11), std::invalid_argument);
    QVERIFY_EXCEPTION_THROWN(map.addAccessPoint(2, 2, 2.4, 21), std::invalid_argument);
}

void MapModelTests::barriersAttenuateSignal()
{
    MapModel clear(12, 12);
    MapModel blocked(12, 12);
    clear.addAccessPoint(1, 5, 2.4, 20);
    blocked.addAccessPoint(1, 5, 2.4, 20);
    blocked.addBarrier(5, 0, 5, 10, 30, WallMaterial::Wood);
    const double signalWithoutBarrier = clear.parcelAt(9, 5).signalDbm;
    const double signalWithBarrier = blocked.parcelAt(9, 5).signalDbm;
    QVERIFY(signalWithBarrier < signalWithoutBarrier);
    QVERIFY(std::abs((signalWithoutBarrier - signalWithBarrier) - 15.0) < 0.01);
    QVERIFY_EXCEPTION_THROWN(blocked.addBarrier(2, 2, 4, 3, 10, WallMaterial::Brick),
                             std::invalid_argument);
}

void MapModelTests::resizePreservesElements()
{
    MapModel map(20, 20);
    map.addAccessPoint(12, 12, 2.4, 14);
    QVERIFY_EXCEPTION_THROWN(map.resize(10, 20), std::invalid_argument);
    QCOMPARE(map.length(), 20);
    map.resize(15, 15);
    QCOMPARE(map.length(), 15);
    QCOMPARE(map.accessPoints().size(), 1);
}

void MapModelTests::jsonRoundTripPreservesMap()
{
    MapModel source(25, 16);
    const int pointId = source.addAccessPoint(4, 5, 5.0, 19);
    const int barrierId = source.addBarrier(12, 1, 12, 10, 25, WallMaterial::Brick);
    MapModel restored;
    restored.loadJson(source.toJson());
    QCOMPARE(restored.length(), 25);
    QCOMPARE(restored.width(), 16);
    QCOMPARE(restored.accessPoints().first().id, pointId);
    QCOMPARE(restored.barriers().first().id, barrierId);
    QCOMPARE(restored.barriers().first().material, WallMaterial::Brick);
    const int nextId = restored.addAccessPoint(20, 12, 2.4, 11);
    QVERIFY(nextId > barrierId);
}

void MapModelTests::invalidJsonDoesNotChangeMap()
{
    MapModel map(20, 20);
    map.addAccessPoint(4, 4, 2.4, 15);
    QJsonObject invalid = map.toJson();
    invalid.insert(QStringLiteral("length"), 0);
    QVERIFY_EXCEPTION_THROWN(map.loadJson(invalid), std::invalid_argument);
    QCOMPARE(map.length(), 20);
    QCOMPARE(map.accessPoints().size(), 1);
}

QTEST_APPLESS_MAIN(MapModelTests)
#include "test_mapmodel.moc"
