# navigation_2d
Developer : Mahdi Nejadsahebi.

This is a simple app to navigate between the ways.

[Test the CDijkstra class]
Test1
##########
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
    Output
    
    Shortest path has 5.56776 distance and 3 points:
    (0, 0, 0)
    (5, 1, 0)
    (7, 0, 0)
##########
    
 Test2
##########
    ...
    dijkstra.FindShortestPath(d, c, pathDistance);//Succeeded
    ---------
    Output
    
  Shortest path has 5.65685 distance and 4 points:
  (3, 2, 0)
  (4, 5, 0)
  (5, 1, 0)
  (7, 0, 0)
##########

Test3
##########
    ...
    dijkstra.FindShortestPath(c, f, pathDistance);//Succeeded
    ---------
    Output
    
  Shortest path has 0 distance and 0 points:
##########

Test4
##########
    ...
    dijkstra.FindShortestPath(e, a, pathDistance);//Succeeded
    ---------
    Output
    
    Shortest path has 4.79583 distance and 3 points:
    (4, 5, 0)
    (3, 2, 0)
    (0, 0, 0)
##########

Test5
##########
    ...
    dijkstra.FindShortestPath(e, f, pathDistance);//Succeeded
    ---------
    Output
    
    Shortest path has 7.4162 distance and 3 points:
    (4, 5, 0)
    (3, 2, 0)
    (9, 5, 0)
##########

Test6
##########
    ...
    dijkstra.FindShortestPath(e, b, pathDistance);//Succeeded
    ---------
    Output
    
    Shortest path has 4.12311 distance and 2 points:
    (4, 5, 0)
    (5, 1, 0)
##########


       
