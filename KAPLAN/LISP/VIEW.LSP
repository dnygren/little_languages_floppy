(defun view (referent)  (process referent))

(defun process (referent)
	(cond 	((equal (car referent) 'and)
		 (andimagelist
			(process (cadr referent))
			(process (caddr referent))))
		((equal (car referent) 'or)
		 (orimagelist
			(process (cadr referent))
			(process (caddr referent))))
		(t (processelement referent))))

(defun andimagelist (list1 list2)
  (cond ((null list1) nil)
	((member (car list1) list2)
	 (cons 	(car list1)
		(andimagelist (cdr list1) list2)))
	(t (andimagelist (cdr list1) list2))))


(defun orimagelist (list1 list2)
  (cond ((null list1) list2)
	((not (member (car list1) list2))
	 (orimagelist (cdr list1)
		      (append list2 (list (car list1)))))
   (t (orimagelist (cdr list1) list2))))

 
(setq imagedb    
'((image792	image	((house large) (color beige)
			 (bedrooms 4) (bathrooms 3)
			 (type colonial)))
 (image334	image	((house small) (color white)
			 (bedrooms 3) (bathrooms 2)
			 (type colonial)))
 (image412	image	((house large) (color white)
			 (bedrooms 3) (bathrooms 2)
			 (type modern)))
 (image257	image	((house large) (color beige)
			 (bedrooms 5) (bathrooms 3)
			 (type modern)))))

     
(defun processelement (referent)
	(mapcan #'(lambda (x) (processimage x referent)) imagedb))

(defun processimage (image referent)
	(let (result)
		(setq result (assoc (car referent) (caddr image)))
		(if (equal (cadr referent) (cadr result)) (list (car image)) nil)))
