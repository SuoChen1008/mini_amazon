
1. When a new purchase request created at the front-end, it should notify back-end.
So we created a socket for communications between front-end and back-end. Front-end will first store the package into database and then notify the daemon by sending the id.


2. Users that are not login shouldn't get access to purchasing, modifying profile and other web pages.
So we added "@login_required" for related functions in views.py and fixed other model-related bugs.



3. When we checkInventory, we need to consider the large concurrent requests, so here, we add optimistic concurrency control to check the order's version, if the
version is not our expected, we will do nothing for it. So we can make sure the security for the database data


4. When we send Amazon Command or UACommand, we consider the concurrent problems, so we create thread safe queue data structure to solve it.

