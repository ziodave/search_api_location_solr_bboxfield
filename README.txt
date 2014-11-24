Search API Location - Solr BBoxField
------------------------------------

This module adds support for the "BBoxField" field type introduced in Solr 4.10
to Search API Location Views.

Information for users
---------------------

To use the functionality this module provides, you will need an Apache Solr
server installed of version 4.10 or higher. Then, on any index lying on such a
server, you can index the "Well-known text" property of any Geofields with the
new data type "Location area (WKT)". (You first have to add the sub-properties
of that field with "Add related fields" at the bottom of the form.)

You will then have specific options and functionality when using this field as a
filter in Views.

For this to work, you also need to place the two schema_extra_*.xml files from
this module into Solr's config folder, overwriting the ones provided by the
Search API Solr Search module. Don't forget to restart the Solr server after
making this change.

Information for developers
--------------------------

This module provides a new datatype, "search_api_location_solr_bboxfield", with
corresponding feature "search_api_data_type_search_api_location_solr_bboxfield".
By supporting this feature with your service class, you indicate that you can
index the data type in a useful manner. The data type is defined as a string
describing an area in Well-known Text (WKT) format.

In addition to being able to index locations, you should also recognize the
"search_api_location_solr_bboxfield" search query option, which is defined as
follows:

The option is an array, where each value is an array that defines one set of
location area data for the query and has the following structure:
- field: The Search API field identifier of the location field. Must be indexed
  as type "location_area".
- bbox: A rectangle ("bounding box") for which to filter. The rectangle is
  represented as an associative array containing exactly the following keys:
  - left
  - bottom
  - right
  - top
- operator: (optional) The operator to use for finding matches with the bounding
  box. One of "Intersects", "IsWithin", "Contains" or "Disjoint". Defaults to
  "Intersects".
