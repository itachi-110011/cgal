#include <CGAL/Linear_cell_complex_for_combinatorial_map.h>
#include <CGAL/Linear_cell_complex_constructors.h>
#include <CGAL/Surface_mesh_curve_topology.h>
#include <CGAL/Path_on_surface.h>
#include <CGAL/draw_lcc_with_paths.h>

typedef CGAL::Linear_cell_complex_for_combinatorial_map<2,3> LCC_3_cmap;

///////////////////////////////////////////////////////////////////////////////
void create_path_1(CGAL::Path_on_surface<LCC_3_cmap>& p)
{
  p.push_back_by_index(56); // Its starting dart
  for (int i=0; i<3; ++i)
  { p.extend_positive_turn(2); } // Extend the path
}
///////////////////////////////////////////////////////////////////////////////
void create_path_2(CGAL::Path_on_surface<LCC_3_cmap>& p)
{
  p.push_back_by_index(202);  // Its starting dart
  for (int i=0; i<3; ++i)
  { p.extend_negative_turn(2); } // Extend the path
}
///////////////////////////////////////////////////////////////////////////////
void create_path_3(CGAL::Path_on_surface<LCC_3_cmap>& p)
{
  p.push_back_by_index(411); // Its starting dart
  p.extend_positive_turn(1); // Extend the path
  for (int i=0; i<3; ++i)
  { p.extend_positive_turn(2); }
  p.extend_positive_turn(1);
}
///////////////////////////////////////////////////////////////////////////////
int main()
{
  LCC_3_cmap lcc;
  if (!CGAL::load_off(lcc, "data/double-torus-example.off"))
  {
    std::cout<<"ERROR reading file data/double-torus-example.off"<<std::endl;
    exit(EXIT_FAILURE);
  }

  CGAL::Surface_mesh_curve_topology<LCC_3_cmap> smct(lcc);
  CGAL::Path_on_surface<LCC_3_cmap> p1(lcc), p2(lcc), p3(lcc);
  create_path_1(p1);
  create_path_2(p2);
  create_path_3(p3);

  bool res1=smct.are_base_point_homotopic(p1, p2);
  std::cout<<"Path p1 (pink) "<<(res1?"IS":"IS NOT")
           <<" base point homotopic with path p2 (green)."<<std::endl;

  bool res2=smct.are_base_point_homotopic(p1, p3);
  std::cout<<"Path p1 (pink) "<<(res2?"IS":"IS NOT")
           <<" base point homotopic with path p3 (orange)."<<std::endl;

#ifdef CGAL_USE_BASIC_VIEWER
  std::vector<CGAL::Path_on_surface<LCC_3_cmap> > paths;
  paths.push_back(p1);
  paths.push_back(p2);  
  paths.push_back(p3);  
  CGAL::draw(lcc, paths); // Enable only if CGAL was compiled with Qt5
#endif // CGAL_USE_BASIC_VIEWER
  
  return EXIT_SUCCESS;
}
///////////////////////////////////////////////////////////////////////////////
