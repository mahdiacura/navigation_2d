# navigation_3d
Developer : Mahdi Nejadsahebi.

This is a simple app to navigate between the ways with real coordinates;

# Requirements

>Add source codes below to project :

Coordinate	:

	cbox.h,		ccoordinate.h,		cway.h
    
	cbox.cpp,	ccoordinate.cpp,	cway.cpp
    
Dijkstra	:

	cdijkstra.h,	csystemversion.h
    
	cdijkstra.cpp
    
ShapeLib	:

	shapefil.h,	shpgeo.h
    
	dbfopen.c,	shpgeo.cpp,	safileio.c,	sbnsearch.h,	shpopen.c,	shptree.c
    
OpenGL  	:

	openglwindow.h
    
	openglwindow.cpp


# How To Use

- Create an instance of CDijkstra class.
- Load the shape file with instance.LoadShapeFile() function.
- Get the shortest path between two coordinates coordinate1 & coordinate2 by 	instance.FindShortestPath() function.

[Test the CDijkstra class]
Test1 - Add coordinates manually
```
    CCoordinate a(0, 0, 0), b(5, 1, 0), c(7, 0, 0), d(3, 2, 0), e(4, 5, 0), f(9, 5, 0);
    CDijkstra dijkstra;
    dijkstra.AddWay(CWay(a, b, true));
    dijkstra.AddWay(CWay(a, d, false));
    dijkstra.AddWay(CWay(b, c, true));
    dijkstra.AddWay(CWay(d, e, false));
    dijkstra.AddWay(CWay(e, c, true));
    dijkstra.AddWay(CWay(d, f, false));
    dijkstra.AddWay(CWay(f, c, true));
    dijkstra.AddWay(CWay(e, b, true));
    dijkstra.GenerateDistancesMatrix(6);
    
    double pathDistance = 0;
    std::vector<CCoordinate> shortestPath =
         dijkstra.FindShortestPath(d, c, pathDistance);//Succeeded
    std::cout << "\nShortest path has " << pathDistance << " distance and " << shortestPath.size() << " points:\n";
    for (std::vector<CCoordinate>::iterator coordinates = shortestPath.begin();
         coordinates != shortestPath.end();
         coordinates++){
        std::cout << "(" <<
            coordinates->m_x << ", " <<
            coordinates->m_y << ", " <<
            coordinates->m_z << ")\n";
    }
    std::cout.flush();
    
    ---------
    Output in console
    
    Shortest path has 5.56776 distance and 3 points:
    (3, 2, 0)
    (4, 5, 0)
    (5, 1, 0)
    (7, 0, 0)
```
    
 Test2 - Add coordinates by loading the shapefile

[Draw shortest path with OpenGL]

openglwindow.cpp

```
CDijkstra dijkstra ;

CCoordinate sourceCoordinate		(51.359152, 35.773133, 0);
CCoordinate destinationCoordinate	(51.3774031, 35.7743919, 0);
std::vector<CCoordinate> shortestPath;
std::vector<int8_t> pathDirections;
double_t pathDistance = 0;

dijkstra.LoadShapeFile("E:/Workstation/C++/NavigationTest2D/Data/test/test.shp");
shortestPath =		dijkstra.FindShortestPath(
				sourceCoordinate,
				destinationCoordinate,
				pathDistance,
				pathDirections);

```

And also you can route by pressing down clik (Set the start coordinate) and move the cursor, then release click (To set the destination coordinate). The result will be drawed in the opengl and specefics of shortest path in the shortest_path.txt like :


```
[Shortest Path]
Distance : 275.323055m

1th way		- Length : 19.545137m		- Direction : Go Straight
2th way		- Length : 19.588710m		- Direction : Go Straight
3th way		- Length : 63.597656m		- Direction : Go Straight
4th way		- Length : 53.421747m		- Direction : Turn Right
5th way		- Length : 52.857154m		- Direction : Go Straight
6th way		- Length : 6.348822m		- Direction : Turn Left
7th way		- Length : 59.963830m		- Direction : Go Straight
```

[Benefits]
1_ Coordinates and distances are calculated in 3D space.

2_ Searches distances as R^2 instead of R for performance.

3_ Distances between each coordinates and also whole of shortest path is calculated on earth(Longitude, Latitude)(Real distance) in meters.

4_ You can pin and select coordinates by FindNearest() to find a coorinate when you select a point at it's neighborhood (Suitable for apps).

5_ Direction of each coordinate has been calculated for the previous coordinate.


Route Sample

![Route1](https://github.com/mahdiacura/navigation_2d/blob/version1.3/Data/RoutesPreview/1.jpg)
