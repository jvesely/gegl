#include <glib-object.h>
#include "gegl.h"
#include "gegl-mock-node-visitor.h"
#include "gegl-mock-operation-0-1.h"
#include "gegl-mock-operation-1-1.h"
#include "gegl-mock-operation-1-2.h"
#include "gegl-mock-operation-2-1.h"
#include "gegl-mock-operation-2-2.h"
#include "ctest.h"
#include "csuite.h"
#include <string.h>


static gboolean
do_visitor_and_check_visit_order(gchar **visit_order,
                                 gint length,
                                 GeglNode *node)
{
  gint i;
  GList *visits_list = NULL;
  GeglVisitor *  visitor = g_object_new(GEGL_TYPE_MOCK_NODE_VISITOR, NULL);
  gegl_visitor_dfs_traverse(visitor, GEGL_VISITABLE(node));

  visits_list = gegl_visitor_get_visits_list(visitor);

  if(length != (gint)g_list_length(visits_list))
      return FALSE;

  for(i = 0; i < length; i++)
    {
      GeglNode *node = (GeglNode*)g_list_nth_data(visits_list, i);
      const gchar *node_name = gegl_object_get_name(GEGL_OBJECT(node));

      if(0 != strcmp(node_name, visit_order[i]))
        {
          g_object_unref(visitor);
          return FALSE;
        }
    }

  g_object_unref(visitor);
  return TRUE;
}

static void
test_dfs_node_visitor(Test *test)
{
  /*
       -
       C
      + +
      | |
      - -
      A B
  */

  {
    gchar * visit_order[] = { "A",
                              "B",
                              "C" };

    GeglNode *A = g_object_new (GEGL_TYPE_NODE, "name", "A", "operation", "GeglMockOperation01", NULL);
    GeglNode *B = g_object_new (GEGL_TYPE_NODE, "name", "B", "operation", "GeglMockOperation01", NULL);
    GeglNode *C = g_object_new (GEGL_TYPE_NODE, "name", "C", "operation", "GeglMockOperation21", NULL);

    gegl_node_connect_from(C, "input0", A, "output0");
    gegl_node_connect_from(C, "input1", B, "output0");

    ct_test(test, do_visitor_and_check_visit_order(visit_order, 3, C));

    g_object_unref(A);
    g_object_unref(B);
    g_object_unref(C);
  }

  /*
     -
     C
     +
     |
     -
     B
     +
     |
     -
     A
  */
  {
    gchar * visit_order[] = { "A",
                              "B",
                              "C" };

    GeglNode *A = g_object_new (GEGL_TYPE_NODE, "name", "A", "operation", "GeglMockOperation01", NULL);
    GeglNode *B = g_object_new (GEGL_TYPE_NODE, "name", "B", "operation", "GeglMockOperation11", NULL);
    GeglNode *C = g_object_new (GEGL_TYPE_NODE, "name", "C", "operation", "GeglMockOperation11", NULL);

    gegl_node_connect_from(B, "input0", A, "output0");
    gegl_node_connect_from(C, "input0", B, "output0");

    ct_test(test, do_visitor_and_check_visit_order(visit_order, 3, C));

    g_object_unref(A);
    g_object_unref(B);
    g_object_unref(C);
  }

  /*
       -
       B
      + +
      \ /
       -
       A
  */

  {
    gchar * visit_order[] = { "A",
                              "B" };

    GeglNode *A = g_object_new (GEGL_TYPE_NODE, "name", "A", "operation", "GeglMockOperation01", NULL);
    GeglNode *B = g_object_new (GEGL_TYPE_NODE, "name", "B", "operation", "GeglMockOperation21", NULL);

    gegl_node_connect_from(B, "input0", A, "output0");
    gegl_node_connect_from(B, "input1", A, "output0");

    ct_test(test, do_visitor_and_check_visit_order(visit_order, 2, B));

    g_object_unref(A);
    g_object_unref(B);
  }


  /*
       -
       C
      + +
        /
       -
       B
      + +
      \
       -
       A
  */

  {
    gchar * visit_order[] = { "A",
                              "B",
                              "C" };

    GeglNode *A = g_object_new (GEGL_TYPE_NODE, "name", "A", "operation", "GeglMockOperation01", NULL);
    GeglNode *B = g_object_new (GEGL_TYPE_NODE, "name", "B", "operation", "GeglMockOperation21", NULL);
    GeglNode *C = g_object_new (GEGL_TYPE_NODE, "name", "C", "operation", "GeglMockOperation21", NULL);

    gegl_node_connect_from(B, "input0", A, "output0");
    gegl_node_connect_from(C, "input1", B, "output0");

    ct_test(test, do_visitor_and_check_visit_order(visit_order, 3, C));

    g_object_unref(A);
    g_object_unref(B);
    g_object_unref(C);
  }


  /*
       -
       C
      + +
      | |
      - -
       B
      + +
      \ /
       -
       A
  */

  {
    gchar * visit_order[] = { "A",
                              "B",
                              "C" };

    GeglNode *A = g_object_new (GEGL_TYPE_NODE, "name", "A", "operation", "GeglMockOperation01", NULL);
    GeglNode *B = g_object_new (GEGL_TYPE_NODE, "name", "B", "operation", "GeglMockOperation22", NULL);
    GeglNode *C = g_object_new (GEGL_TYPE_NODE, "name", "C", "operation", "GeglMockOperation21", NULL);

    gegl_node_connect_from(B, "input0", A, "output0");
    gegl_node_connect_from(B, "input1", A, "output0");
    gegl_node_connect_from(C, "input0", B, "output0");
    gegl_node_connect_from(C, "input1", B, "output1");

    ct_test(test, do_visitor_and_check_visit_order(visit_order, 3, C));

    g_object_unref(A);
    g_object_unref(B);
    g_object_unref(C);
  }


  /*
       -
       B
      + +
      | |
      - -
       A
       +
  */

  {
    gchar * visit_order[] = { "A",
                              "B" };

    GeglNode *A = g_object_new (GEGL_TYPE_NODE, "name", "A", "operation", "GeglMockOperation12", NULL);
    GeglNode *B = g_object_new (GEGL_TYPE_NODE, "name", "B", "operation", "GeglMockOperation21", NULL);

    gegl_node_connect_from(B, "input0", A, "output0");
    gegl_node_connect_from(B, "input1", A, "output1");

    ct_test(test, do_visitor_and_check_visit_order(visit_order, 2, B));

    g_object_unref(A);
    g_object_unref(B);
  }

  /*
      -
      C
     + +
     | |
     | -
     | B
     | +
     \ /
      -
      A
  */

  {
    gchar * visit_order[] = { "A",
                              "B",
                              "C" };

    GeglNode *A = g_object_new (GEGL_TYPE_NODE, "name", "A", "operation", "GeglMockOperation01", NULL);
    GeglNode *B = g_object_new (GEGL_TYPE_NODE, "name", "B", "operation", "GeglMockOperation11", NULL);
    GeglNode *C = g_object_new (GEGL_TYPE_NODE, "name", "C", "operation", "GeglMockOperation21", NULL);

    gegl_node_connect_from(B, "input0", A, "output0");
    gegl_node_connect_from(C, "input1", B, "output0");
    gegl_node_connect_from(C, "input0", A, "output0");

    ct_test(test, do_visitor_and_check_visit_order(visit_order, 3, C));

    g_object_unref(A);
    g_object_unref(B);
    g_object_unref(C);
  }

  /*
      -
      D
     + +
     | |
     | -
     | C
     | +
     \ /
      -
      B
      +
      |
      -
      A
      +

  */

  {
    gchar * visit_order[] = { "A",
                              "B",
                              "C",
                              "D" };

    GeglNode *A = g_object_new (GEGL_TYPE_NODE, "name", "A", "operation", "GeglMockOperation11", NULL);
    GeglNode *B = g_object_new (GEGL_TYPE_NODE, "name", "B", "operation", "GeglMockOperation11", NULL);
    GeglNode *C = g_object_new (GEGL_TYPE_NODE, "name", "C", "operation", "GeglMockOperation11", NULL);
    GeglNode *D = g_object_new (GEGL_TYPE_NODE, "name", "D", "operation", "GeglMockOperation21", NULL);

    gegl_node_connect_from(B, "input0", A, "output0");
    gegl_node_connect_from(C, "input0", B, "output0");
    gegl_node_connect_from(D, "input0", B, "output0");
    gegl_node_connect_from(D, "input1", C, "output0");

    ct_test(test, do_visitor_and_check_visit_order(visit_order, 4, D));

    g_object_unref(A);
    g_object_unref(B);
    g_object_unref(C);
    g_object_unref(D);
  }

  /*
      -
      D
     + +
     | |
     | -
     | C
     | +
     | |
     | -
     | B
     | +
     \ /
      -
      A
  */

  {
    gchar * visit_order[] = { "A",
                              "B",
                              "C",
                              "D" };

    GeglNode *A = g_object_new (GEGL_TYPE_NODE, "name", "A", "operation", "GeglMockOperation01", NULL);
    GeglNode *B = g_object_new (GEGL_TYPE_NODE, "name", "B", "operation", "GeglMockOperation11", NULL);
    GeglNode *C = g_object_new (GEGL_TYPE_NODE, "name", "C", "operation", "GeglMockOperation11", NULL);
    GeglNode *D = g_object_new (GEGL_TYPE_NODE, "name", "D", "operation", "GeglMockOperation21", NULL);

    gegl_node_connect_from(B, "input0", A, "output0");
    gegl_node_connect_from(C, "input0", B, "output0");
    gegl_node_connect_from(D, "input1", C, "output0");
    gegl_node_connect_from(D, "input0", A, "output0");

    ct_test(test, do_visitor_and_check_visit_order(visit_order, 4, D));

    g_object_unref(A);
    g_object_unref(B);
    g_object_unref(C);
    g_object_unref(D);
  }

/*
         E
        / \
       D   F
      /\    \
     B  C    G
     | /
     A

 */

  {
    gchar * visit_order[] = { "A",
                              "B",
                              "C",
                              "D",
                              "G",
                              "F",
                              "E" };

    GeglNode *A = g_object_new (GEGL_TYPE_NODE, "name", "A", "operation", "GeglMockOperation01", NULL);
    GeglNode *B = g_object_new (GEGL_TYPE_NODE, "name", "B", "operation", "GeglMockOperation11", NULL);
    GeglNode *C = g_object_new (GEGL_TYPE_NODE, "name", "C", "operation", "GeglMockOperation11", NULL);
    GeglNode *D = g_object_new (GEGL_TYPE_NODE, "name", "D", "operation", "GeglMockOperation21", NULL);
    GeglNode *E = g_object_new (GEGL_TYPE_NODE, "name", "E", "operation", "GeglMockOperation21", NULL);
    GeglNode *F = g_object_new (GEGL_TYPE_NODE, "name", "F", "operation", "GeglMockOperation11", NULL);
    GeglNode *G = g_object_new (GEGL_TYPE_NODE, "name", "G", "operation", "GeglMockOperation01", NULL);

    gegl_node_connect_from(B, "input0", A, "output0");
    gegl_node_connect_from(D, "input0", B, "output0");
    gegl_node_connect_from(C, "input0", A, "output0");
    gegl_node_connect_from(D, "input1", C, "output0");
    gegl_node_connect_from(F, "input0", G, "output0");
    gegl_node_connect_from(E, "input0", D, "output0");
    gegl_node_connect_from(E, "input1", F, "output0");

    ct_test(test, do_visitor_and_check_visit_order(visit_order, 7, E));

    g_object_unref(A);
    g_object_unref(B);
    g_object_unref(C);
    g_object_unref(D);
    g_object_unref(E);
    g_object_unref(F);
    g_object_unref(G);
  }
}

static void
dfs_node_visitor_test_setup(Test *test)
{
}

static void
dfs_node_visitor_test_teardown(Test *test)
{
}

Test *
create_dfs_node_visitor_test()
{
  Test* t = ct_create("GeglDfsNodeVisitorTest");

  g_assert(ct_addSetUp(t, dfs_node_visitor_test_setup));
  g_assert(ct_addTearDown(t, dfs_node_visitor_test_teardown));

#if 1
  g_assert(ct_addTestFun(t, test_dfs_node_visitor));
#endif

  return t;
}
